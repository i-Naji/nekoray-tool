
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QJsonArray>
#include <QJsonDocument>
#include <QCoreApplication>
#include <QApplication>
#include <QDir>
#include <yaml-cpp/yaml.h>
#include "tools/client.hpp"
#include "db/ConfigBuilder.hpp"
#include "db/Database.hpp"
#include "db/ProfileFilter.hpp"
#include "fmt/includes.h"
#include "fmt/Preset.hpp"
#include "main/QJS.hpp"
#include "main/HTTPRequestHelper.hpp"
#include <iostream>
#include <thread>
#include "main/NekoGui.hpp"
#include "sys/ExternalProcess.hpp"
#include <QMainWindow>
#include "go/grpc_server/gen/libcore.grpc.pb.h"
#include <grpcpp/grpcpp.h>


using grpc::Channel;
using NekoGui::ProfileManager; using NekoGui::ProxyEntity; using NekoGui::BuildConfig; using NekoGui::dataStore; using NekoGui::Routing;
using NekoGui_sys::ExternalProcess;
using NekoGui_fmt::GetStreamSettings;

NekoGui_sys::CoreProcess *core_process;
libcoreClient *defaultClient;

#define EXPORT __attribute__ ((visibility ("default")))

extern "C" {

void _fix_ent(const std::shared_ptr<NekoGui::ProxyEntity> &ent) {
    if (ent == nullptr) return;
    auto stream = NekoGui_fmt::GetStreamSettings(ent->bean.get());
    if (stream == nullptr) return;
    // 1. "security"
    if (stream->security == "none" || stream->security == "0" || stream->security == "false") {
        stream->security = "";
    } else if (stream->security == "1" || stream->security == "true") {
        stream->security = "tls";
    }
    // 2. TLS SNI: v2rayN config builder generate sni like this, so set sni here for their format.
    if (stream->security == "tls" && IsIpAddress(ent->bean->serverAddress) && (!stream->host.isEmpty()) && stream->sni.isEmpty()) {
        stream->sni = stream->host;
    }
}



QString Node2QString(const YAML::Node &n, const QString &def = "") {
    try {
        return n.as<std::string>().c_str();
    } catch (const YAML::Exception &ex) {
        qDebug() << ex.what();
        return def;
    }
}

QStringList Node2QStringList(const YAML::Node &n) {
    try {
        if (n.IsSequence()) {
            QStringList list;
            for (auto item: n) {
                list << item.as<std::string>().c_str();
            }
            return list;
        } else {
            return {};
        }
    } catch (const YAML::Exception &ex) {
        qDebug() << ex.what();
        return {};
    }
}

int Node2Int(const YAML::Node &n, const int &def = 0) {
    try {
        return n.as<int>();
    } catch (const YAML::Exception &ex) {
        qDebug() << ex.what();
        return def;
    }
}

bool Node2Bool(const YAML::Node &n, const bool &def = false) {
    try {
        return n.as<bool>();
    } catch (const YAML::Exception &ex) {
        try {
            return n.as<int>();
        } catch (const YAML::Exception &ex2) {
            ex2.what();
        }
        qDebug() << ex.what();
        return def;
    }
}

// NodeChild returns the first defined children or Null Node
YAML::Node NodeChild(const YAML::Node &n, const std::list<std::string> &keys) {
    for (const auto &key: keys) {
        auto child = n[key];
        if (child.IsDefined()) return child;
    }
    return {};
}

void _extract_clash(const QString &str, std::vector<std::shared_ptr<NekoGui::ProxyEntity>> &ents) {
    try {
        auto proxies = YAML::Load(str.toStdString())["proxies"];
        for (auto proxy: proxies) {
            auto type = Node2QString(proxy["type"]).toLower();
            auto type_clash = type;

            if (type == "ss" || type == "ssr") type = "shadowsocks";
            if (type == "socks5") type = "socks";

            auto ent = NekoGui::ProfileManager::NewProxyEntity(type);
            if (ent->bean->version == -114514) continue;
            bool needFix = false;

            // common
            ent->bean->name = Node2QString(proxy["name"]);
            ent->bean->serverAddress = Node2QString(proxy["server"]);
            ent->bean->serverPort = Node2Int(proxy["port"]);

            if (type_clash == "ss") {
                auto bean = ent->ShadowSocksBean();
                bean->method = Node2QString(proxy["cipher"]).replace("dummy", "none");
                bean->password = Node2QString(proxy["password"]);
                auto plugin_n = proxy["plugin"];
                auto pluginOpts_n = proxy["plugin-opts"];

                // UDP over TCP
                if (Node2Bool(proxy["udp-over-tcp"])) {
                    bean->uot = Node2Int(proxy["udp-over-tcp-version"]);
                    if (bean->uot == 0) bean->uot = 2;
                }

                if (plugin_n.IsDefined() && pluginOpts_n.IsDefined()) {
                    QStringList ssPlugin;
                    auto plugin = Node2QString(plugin_n);
                    if (plugin == "obfs") {
                        ssPlugin << "obfs-local";
                        ssPlugin << "obfs=" + Node2QString(pluginOpts_n["mode"]);
                        ssPlugin << "obfs-host=" + Node2QString(pluginOpts_n["host"]);
                    } else if (plugin == "v2ray-plugin") {
                        auto mode = Node2QString(pluginOpts_n["mode"]);
                        auto host = Node2QString(pluginOpts_n["host"]);
                        auto path = Node2QString(pluginOpts_n["path"]);
                        ssPlugin << "v2ray-plugin";
                        if (!mode.isEmpty() && mode != "websocket") ssPlugin << "mode=" + mode;
                        if (Node2Bool(pluginOpts_n["tls"])) ssPlugin << "tls";
                        if (!host.isEmpty()) ssPlugin << "host=" + host;
                        if (!path.isEmpty()) ssPlugin << "path=" + path;
                        // clash only: skip-cert-verify
                        // clash only: headers
                        // clash: mux=?
                    }
                    bean->plugin = ssPlugin.join(";");
                }

                // sing-mux
                auto smux = NodeChild(proxy, {"smux"});
                if (Node2Bool(smux["enabled"])) bean->stream->multiplex_status = 1;
            } else if (type == "socks" || type == "http") {
                auto bean = ent->SocksHTTPBean();
                bean->username = Node2QString(proxy["username"]);
                bean->password = Node2QString(proxy["password"]);
                if (Node2Bool(proxy["tls"])) bean->stream->security = "tls";
                if (Node2Bool(proxy["skip-cert-verify"])) bean->stream->allow_insecure = true;
            } else if (type == "trojan" || type == "vless") {
                needFix = true;
                auto bean = ent->TrojanVLESSBean();
                if (type == "vless") {
                    bean->flow = Node2QString(proxy["flow"]);
                    bean->password = Node2QString(proxy["uuid"]);
                    // meta packet encoding
                    if (Node2Bool(proxy["packet-addr"])) {
                        bean->stream->packet_encoding = "packetaddr";
                    } else {
                        // For VLESS, default to use xudp
                        bean->stream->packet_encoding = "xudp";
                    }
                } else {
                    bean->password = Node2QString(proxy["password"]);
                }
                bean->stream->security = "tls";
                bean->stream->network = Node2QString(proxy["network"], "tcp");
                bean->stream->sni = FIRST_OR_SECOND(Node2QString(proxy["sni"]), Node2QString(proxy["servername"]));
                bean->stream->alpn = Node2QStringList(proxy["alpn"]).join(",");
                bean->stream->allow_insecure = Node2Bool(proxy["skip-cert-verify"]);
                bean->stream->utlsFingerprint = Node2QString(proxy["client-fingerprint"]);
                if (bean->stream->utlsFingerprint.isEmpty()) {
                    bean->stream->utlsFingerprint = NekoGui::dataStore->utlsFingerprint;
                }

                // sing-mux
                auto smux = NodeChild(proxy, {"smux"});
                if (Node2Bool(smux["enabled"])) bean->stream->multiplex_status = 1;

                // opts
                auto ws = NodeChild(proxy, {"ws-opts", "ws-opt"});
                if (ws.IsMap()) {
                    auto headers = ws["headers"];
                    for (auto header: headers) {
                        if (Node2QString(header.first).toLower() == "host") {
                            bean->stream->host = Node2QString(header.second);
                        }
                    }
                    bean->stream->path = Node2QString(ws["path"]);
                    bean->stream->ws_early_data_length = Node2Int(ws["max-early-data"]);
                    bean->stream->ws_early_data_name = Node2QString(ws["early-data-header-name"]);
                }

                auto grpc = NodeChild(proxy, {"grpc-opts", "grpc-opt"});
                if (grpc.IsMap()) {
                    bean->stream->path = Node2QString(grpc["grpc-service-name"]);
                }

                auto reality = NodeChild(proxy, {"reality-opts"});
                if (reality.IsMap()) {
                    bean->stream->reality_pbk = Node2QString(reality["public-key"]);
                    bean->stream->reality_sid = Node2QString(reality["short-id"]);
                }
            } else if (type == "vmess") {
                needFix = true;
                auto bean = ent->VMessBean();
                bean->uuid = Node2QString(proxy["uuid"]);
                bean->aid = Node2Int(proxy["alterId"]);
                bean->security = Node2QString(proxy["cipher"], bean->security);
                bean->stream->network = Node2QString(proxy["network"], "tcp").replace("h2", "http");
                bean->stream->sni = FIRST_OR_SECOND(Node2QString(proxy["sni"]), Node2QString(proxy["servername"]));
                bean->stream->alpn = Node2QStringList(proxy["alpn"]).join(",");
                if (Node2Bool(proxy["tls"])) bean->stream->security = "tls";
                if (Node2Bool(proxy["skip-cert-verify"])) bean->stream->allow_insecure = true;
                bean->stream->utlsFingerprint = Node2QString(proxy["client-fingerprint"]);
                bean->stream->utlsFingerprint = Node2QString(proxy["client-fingerprint"]);
                if (bean->stream->utlsFingerprint.isEmpty()) {
                    bean->stream->utlsFingerprint = NekoGui::dataStore->utlsFingerprint;
                }

                // sing-mux
                auto smux = NodeChild(proxy, {"smux"});
                if (Node2Bool(smux["enabled"])) bean->stream->multiplex_status = 1;

                // meta packet encoding
                if (Node2Bool(proxy["xudp"])) bean->stream->packet_encoding = "xudp";
                if (Node2Bool(proxy["packet-addr"])) bean->stream->packet_encoding = "packetaddr";

                // opts
                auto ws = NodeChild(proxy, {"ws-opts", "ws-opt"});
                if (ws.IsMap()) {
                    auto headers = ws["headers"];
                    for (auto header: headers) {
                        if (Node2QString(header.first).toLower() == "host") {
                            bean->stream->host = Node2QString(header.second);
                        }
                    }
                    bean->stream->path = Node2QString(ws["path"]);
                    bean->stream->ws_early_data_length = Node2Int(ws["max-early-data"]);
                    bean->stream->ws_early_data_name = Node2QString(ws["early-data-header-name"]);
                    // for Xray
                    if (Node2QString(ws["early-data-header-name"]) == "Sec-WebSocket-Protocol") {
                        bean->stream->path += "?ed=" + Node2QString(ws["max-early-data"]);
                    }
                }

                auto grpc = NodeChild(proxy, {"grpc-opts", "grpc-opt"});
                if (grpc.IsMap()) {
                    bean->stream->path = Node2QString(grpc["grpc-service-name"]);
                }

                auto h2 = NodeChild(proxy, {"h2-opts", "h2-opt"});
                if (h2.IsMap()) {
                    auto hosts = h2["host"];
                    for (auto host: hosts) {
                        bean->stream->host = Node2QString(host);
                        break;
                    }
                    bean->stream->path = Node2QString(h2["path"]);
                }

                auto tcp_http = NodeChild(proxy, {"http-opts", "http-opt"});
                if (tcp_http.IsMap()) {
                    bean->stream->network = "tcp";
                    bean->stream->header_type = "http";
                    auto headers = tcp_http["headers"];
                    for (auto header: headers) {
                        if (Node2QString(header.first).toLower() == "host") {
                            bean->stream->host = Node2QString(header.second[0]);
                        }
                        break;
                    }
                    auto paths = tcp_http["path"];
                    for (auto path: paths) {
                        bean->stream->path = Node2QString(path);
                        break;
                    }
                }
            } else if (type == "hysteria") {
                auto bean = ent->QUICBean();

                bean->hopPort = Node2QString(proxy["ports"]);

                bean->allowInsecure = Node2Bool(proxy["skip-cert-verify"]);
                auto alpn = Node2QStringList(proxy["alpn"]);
                bean->caText = Node2QString(proxy["ca-str"]);
                if (!alpn.isEmpty()) bean->alpn = alpn[0];
                bean->sni = Node2QString(proxy["sni"]);

                auto auth_str = FIRST_OR_SECOND(Node2QString(proxy["auth_str"]), Node2QString(proxy["auth-str"]));
                auto auth = Node2QString(proxy["auth"]);
                if (!auth_str.isEmpty()) {
                    bean->authPayloadType = NekoGui_fmt::QUICBean::hysteria_auth_string;
                    bean->authPayload = auth_str;
                }
                if (!auth.isEmpty()) {
                    bean->authPayloadType = NekoGui_fmt::QUICBean::hysteria_auth_base64;
                    bean->authPayload = auth;
                }
                bean->obfsPassword = Node2QString(proxy["obfs"]);

                if (Node2Bool(proxy["disable_mtu_discovery"]) || Node2Bool(proxy["disable-mtu-discovery"])) bean->disableMtuDiscovery = true;
                bean->streamReceiveWindow = Node2Int(proxy["recv-window"]);
                bean->connectionReceiveWindow = Node2Int(proxy["recv-window-conn"]);

                auto upMbps = Node2QString(proxy["up"]).split(" ")[0].toInt();
                auto downMbps = Node2QString(proxy["down"]).split(" ")[0].toInt();
                if (upMbps > 0) bean->uploadMbps = upMbps;
                if (downMbps > 0) bean->downloadMbps = downMbps;
            } else if (type == "hysteria2") {
                auto bean = ent->QUICBean();

                // bean->hopPort = Node2QString(proxy["ports"]);

                bean->allowInsecure = Node2Bool(proxy["skip-cert-verify"]);
                bean->caText = Node2QString(proxy["ca-str"]);
                bean->sni = Node2QString(proxy["sni"]);

                bean->obfsPassword = Node2QString(proxy["obfs-password"]);
                bean->password = Node2QString(proxy["password"]);

                bean->uploadMbps = Node2QString(proxy["up"]).split(" ")[0].toInt();
                bean->downloadMbps = Node2QString(proxy["down"]).split(" ")[0].toInt();
            } else if (type == "tuic") {
                auto bean = ent->QUICBean();

                bean->uuid = Node2QString(proxy["uuid"]);
                bean->password = Node2QString(proxy["password"]);

                if (Node2Int(proxy["heartbeat-interval"]) != 0) {
                    bean->heartbeat = Int2String(Node2Int(proxy["heartbeat-interval"])) + "ms";
                }

                bean->udpRelayMode = Node2QString(proxy["udp-relay-mode"], bean->udpRelayMode);
                bean->congestionControl = Node2QString(proxy["congestion-controller"], bean->congestionControl);

                bean->disableSni = Node2Bool(proxy["disable-sni"]);
                bean->zeroRttHandshake = Node2Bool(proxy["reduce-rtt"]);
                bean->allowInsecure = Node2Bool(proxy["skip-cert-verify"]);
                bean->alpn = Node2QStringList(proxy["alpn"]).join(",");
                bean->caText = Node2QString(proxy["ca-str"]);
                bean->sni = Node2QString(proxy["sni"]);

                if (Node2Bool(proxy["udp-over-stream"])) bean->uos = true;

                if (!Node2QString(proxy["ip"]).isEmpty()) {
                    if (bean->sni.isEmpty()) bean->sni = bean->serverAddress;
                    bean->serverAddress = Node2QString(proxy["ip"]);
                }
            } else {
                continue;
            }

            if (needFix) _fix_ent(ent);
            ents.push_back(ent);
        }
    } catch (const YAML::Exception &ex) {
        
    }
}

void _extract(const QString &str, std::vector<std::shared_ptr<NekoGui::ProxyEntity>> &ents){
    // Base64 encoded subscription
    if (auto str2 = DecodeB64IfValid(str); !str2.isEmpty()) {
        _extract(str2, ents);
        return;
    }

    // Clash
    if (str.contains("proxies:")) {
        _extract_clash(str, ents);
        return;
    }

    

    // Multi line
    if (str.count("\n") > 0) {
        auto list = str.split("\n");
        for (const auto &str2: list) {
            _extract(str2.trimmed(), ents);
        }
        return;
    }


    std::shared_ptr<NekoGui::ProxyEntity> ent;
    bool needFix = true;

    // Nekoray format
    if (str.startsWith("nekoray://")) {
        needFix = false;
        auto link = QUrl(str);
        if (!link.isValid()) return;
        ent = NekoGui::ProfileManager::NewProxyEntity(link.host());
        if (ent->bean->version == -114514) return;
        auto j = DecodeB64IfValid(link.fragment().toUtf8(), QByteArray::Base64UrlEncoding);
        if (j.isEmpty()) return;
        ent->bean->FromJsonBytes(j);
    }

    // SOCKS
    if (str.startsWith("socks5://") || str.startsWith("socks4://") ||
        str.startsWith("socks4a://") || str.startsWith("socks://")) {
        ent = NekoGui::ProfileManager::NewProxyEntity("socks");
        auto ok = ent->SocksHTTPBean()->TryParseLink(str);
        if (!ok) return;
    }

    // HTTP
    if (str.startsWith("http://") || str.startsWith("https://")) {
        ent = NekoGui::ProfileManager::NewProxyEntity("http");
        auto ok = ent->SocksHTTPBean()->TryParseLink(str);
        if (!ok) return;
    }

    // ShadowSocks
    if (str.startsWith("ss://")) {
        ent = NekoGui::ProfileManager::NewProxyEntity("shadowsocks");
        auto ok = ent->ShadowSocksBean()->TryParseLink(str);
        if (!ok) return;
    }

    // VMess
    if (str.startsWith("vmess://")) {
        ent = NekoGui::ProfileManager::NewProxyEntity("vmess");
        auto ok = ent->VMessBean()->TryParseLink(str);
        if (!ok) return;
    }

    // VLESS
    if (str.startsWith("vless://")) {
        ent = NekoGui::ProfileManager::NewProxyEntity("vless");
        auto ok = ent->TrojanVLESSBean()->TryParseLink(str);
        if (!ok) return;
    }

    // Trojan
    if (str.startsWith("trojan://")) {
        ent = NekoGui::ProfileManager::NewProxyEntity("trojan");
        auto ok = ent->TrojanVLESSBean()->TryParseLink(str);
        if (!ok) return;
    }

    // Naive
    if (str.startsWith("naive+")) {
        needFix = false;
        ent = NekoGui::ProfileManager::NewProxyEntity("naive");
        auto ok = ent->NaiveBean()->TryParseLink(str);
        if (!ok) return;
    }

    // Hysteria1
    if (str.startsWith("hysteria://")) {
        needFix = false;
        ent = NekoGui::ProfileManager::NewProxyEntity("hysteria");
        auto ok = ent->QUICBean()->TryParseLink(str);
        if (!ok) return;
    }

    // Hysteria2
    if (str.startsWith("hysteria2://") || str.startsWith("hy2://")) {
        needFix = false;
        ent = NekoGui::ProfileManager::NewProxyEntity("hysteria2");
        auto ok = ent->QUICBean()->TryParseLink(str);
        if (!ok) return;
    }

    // TUIC
    if (str.startsWith("tuic://")) {
        needFix = false;
        ent = NekoGui::ProfileManager::NewProxyEntity("tuic");
        auto ok = ent->QUICBean()->TryParseLink(str);
        if (!ok) return;
    }

    if (ent == nullptr) return;

    // Fix
    if (needFix) _fix_ent(ent);

    ents.push_back(ent);
}


void _get_proxies(const QString &_str, std::vector<std::shared_ptr<NekoGui::ProxyEntity>> &_ps){
    auto content = _str.trimmed();
    bool asURL = false;
    if (content.startsWith("http://") || content.startsWith("https://")) {
        // asURL = true;
    }

    // 网络请求
    if (asURL) {
        auto resp = NetworkRequestHelper::HttpGet(content);
        if (!resp.error.isEmpty()) {
            return;
        }
        content = resp.data;
    }


    // hook.js
    auto source = qjs::ReadHookJS();
    if (!source.isEmpty()) {
        qjs::QJS js(source);
        auto js_result = js.EvalFunction("hook.hook_import", content);
        if (content != js_result) {
            MW_show_log("hook.js modified your import content.");
            content = js_result;
        }
    }
    
    _extract(content, _ps);
    
}

EXPORT const char *url_test(const char* str, const char* url, int32_t timeout){
    QString qstr = QString::fromUtf8(str);
    std::vector<std::shared_ptr<NekoGui::ProxyEntity>> proxies;
    _get_proxies(qstr, proxies);

    if (proxies.empty()) return "Error: no proxies";
    
    auto profile = proxies.front();
    auto c = BuildConfig(profile, true, false);
    if (!c->error.isEmpty()){
        qDebug() << c->error;
        return ("Error: " + c->error.toStdString()).c_str();
    }

    
    libcore::TestReq req;
    req.set_mode(libcore::FullTest);
    req.set_timeout(timeout);
    req.set_url(url);

    auto config = new libcore::LoadConfigReq;
    config->set_core_config(QJsonObject2QString(c->coreConfig, true).toStdString());
    req.set_allocated_config(config);

    // req.set_full_in_out(true);
    // req.set_in_address(profile->bean->serverAddress.toStdString());
    

    req.set_full_latency(true);


    // req.set_full_udp_latency(true);


    // req.set_full_speed(true);
    // req.set_full_speed_url(NekoGui::dataStore->test_download_url.toStdString());
    // req.set_full_speed_timeout(NekoGui::dataStore->test_download_timeout);

    // req.set_address(profile->bean->DisplayAddress().toStdString());

    bool rpcOK;
    auto result = defaultClient->Test(&rpcOK, req);
    
    if (!rpcOK) return "Error: failed rpc";
    if (!result.error().empty()){
        qDebug() << QString::fromStdString(result.error());
        return "Error: bad res";
    }
    return result.full_report().c_str();
}

EXPORT const char* get_proxies(const char* str){
    QString qstr = QString::fromUtf8(str);
    std::vector<std::shared_ptr<NekoGui::ProxyEntity>> proxies;
    _get_proxies(qstr, proxies);
    
    QJsonArray ps;
    
    for (int i = 0; i < proxies.size(); i = i + 1) {
        ps.append(proxies[i]->bean->ToShareLink());
    }
    
    return QJsonDocument(ps).toJson(QJsonDocument::Compact).data();
}

EXPORT const char* get_test_proxies(const char* str){
    QString qstr = QString::fromUtf8(str);
    std::vector<std::shared_ptr<NekoGui::ProxyEntity>> proxies;
    _get_proxies(qstr, proxies);
    
    QJsonObject ps;
    
    for (const auto &profile: proxies){
        auto c = BuildConfig(profile, true, false);
        if (!c->error.isEmpty()){
            continue;
        }
        auto addr = profile->bean->serverAddress;
        if (ps.contains(addr)){
            QJsonArray pa = ps.value(addr).toArray();
            pa.append(c->coreConfig);
            ps[addr] = pa;
        } else {
            QJsonArray pa;
            pa.append(c->coreConfig);
            ps[addr] = pa;
        }
        // ps[profile->bean->serverAddress] = c->coreConfig;
    }
    
    // qDebug().noquote() << "JSON: " << QJsonDocument(ps).toJson();
    return QJsonObject2QString(ps, true).toUtf8().data();
}

EXPORT void setup(const char* str){
    // std::string port_;
    // std::string token_;
    // port_ = port;
    // token_ = token;
    NekoGui::coreType = 1;
    if (NekoGui::dataStore->log_level == "warning") NekoGui::dataStore->log_level = "info";
    if (NekoGui::dataStore->mux_protocol.isEmpty()) NekoGui::dataStore->mux_protocol = "h2mux";
    
    auto defaultGroup = NekoGui::ProfileManager::NewGroup();
    defaultGroup->name = "Default";
    NekoGui::profileManager->AddGroup(defaultGroup);


    // Prepare core
    NekoGui::dataStore->core_token = GetRandomString(32);
    NekoGui::dataStore->core_port = MkPort();
    if (NekoGui::dataStore->core_port <= 0) NekoGui::dataStore->core_port = 19810;
    
    QStringList args;
    args.push_back("nekobox");
    args.push_back("-port");
    args.push_back(Int2String(NekoGui::dataStore->core_port));
    args.push_back("-token");
    args.push_back(NekoGui::dataStore->core_token);
    core_process = new NekoGui_sys::CoreProcess(QString::fromStdString(str), args);
    // // Remember last started
    if (NekoGui::dataStore->remember_enable && NekoGui::dataStore->remember_id >= 0) {
        core_process->start_profile_when_core_is_up = NekoGui::dataStore->remember_id;
    }
    // // Setup
    core_process->Start();

    defaultClient = new libcoreClient(
        grpc::CreateChannel("127.0.0.1:" + Int2String(NekoGui::dataStore->core_port).toStdString(), 
        grpc::InsecureChannelCredentials()),
        NekoGui::dataStore->core_token.toStdString() 
    );
    NekoGui::dataStore->routing = std::make_unique<NekoGui::Routing>();
}

}
