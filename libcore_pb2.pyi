from google.protobuf.internal import containers as _containers
from google.protobuf.internal import enum_type_wrapper as _enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Iterable as _Iterable, Mapping as _Mapping, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class TestMode(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []
    TcpPing: _ClassVar[TestMode]
    UrlTest: _ClassVar[TestMode]
    FullTest: _ClassVar[TestMode]

class UpdateAction(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []
    Check: _ClassVar[UpdateAction]
    Download: _ClassVar[UpdateAction]
TcpPing: TestMode
UrlTest: TestMode
FullTest: TestMode
Check: UpdateAction
Download: UpdateAction

class EmptyReq(_message.Message):
    __slots__ = []
    def __init__(self) -> None: ...

class EmptyResp(_message.Message):
    __slots__ = []
    def __init__(self) -> None: ...

class ErrorResp(_message.Message):
    __slots__ = ["error"]
    ERROR_FIELD_NUMBER: _ClassVar[int]
    error: str
    def __init__(self, error: _Optional[str] = ...) -> None: ...

class LoadConfigReq(_message.Message):
    __slots__ = ["core_config", "enable_nekoray_connections", "stats_outbounds"]
    CORE_CONFIG_FIELD_NUMBER: _ClassVar[int]
    ENABLE_NEKORAY_CONNECTIONS_FIELD_NUMBER: _ClassVar[int]
    STATS_OUTBOUNDS_FIELD_NUMBER: _ClassVar[int]
    core_config: str
    enable_nekoray_connections: bool
    stats_outbounds: _containers.RepeatedScalarFieldContainer[str]
    def __init__(self, core_config: _Optional[str] = ..., enable_nekoray_connections: bool = ..., stats_outbounds: _Optional[_Iterable[str]] = ...) -> None: ...

class TestReq(_message.Message):
    __slots__ = ["mode", "timeout", "address", "config", "inbound", "url", "in_address", "full_latency", "full_speed", "full_speed_url", "full_speed_timeout", "full_in_out", "full_udp_latency", "full_nat"]
    MODE_FIELD_NUMBER: _ClassVar[int]
    TIMEOUT_FIELD_NUMBER: _ClassVar[int]
    ADDRESS_FIELD_NUMBER: _ClassVar[int]
    CONFIG_FIELD_NUMBER: _ClassVar[int]
    INBOUND_FIELD_NUMBER: _ClassVar[int]
    URL_FIELD_NUMBER: _ClassVar[int]
    IN_ADDRESS_FIELD_NUMBER: _ClassVar[int]
    FULL_LATENCY_FIELD_NUMBER: _ClassVar[int]
    FULL_SPEED_FIELD_NUMBER: _ClassVar[int]
    FULL_SPEED_URL_FIELD_NUMBER: _ClassVar[int]
    FULL_SPEED_TIMEOUT_FIELD_NUMBER: _ClassVar[int]
    FULL_IN_OUT_FIELD_NUMBER: _ClassVar[int]
    FULL_UDP_LATENCY_FIELD_NUMBER: _ClassVar[int]
    FULL_NAT_FIELD_NUMBER: _ClassVar[int]
    mode: TestMode
    timeout: int
    address: str
    config: LoadConfigReq
    inbound: str
    url: str
    in_address: str
    full_latency: bool
    full_speed: bool
    full_speed_url: str
    full_speed_timeout: int
    full_in_out: bool
    full_udp_latency: bool
    full_nat: bool
    def __init__(self, mode: _Optional[_Union[TestMode, str]] = ..., timeout: _Optional[int] = ..., address: _Optional[str] = ..., config: _Optional[_Union[LoadConfigReq, _Mapping]] = ..., inbound: _Optional[str] = ..., url: _Optional[str] = ..., in_address: _Optional[str] = ..., full_latency: bool = ..., full_speed: bool = ..., full_speed_url: _Optional[str] = ..., full_speed_timeout: _Optional[int] = ..., full_in_out: bool = ..., full_udp_latency: bool = ..., full_nat: bool = ...) -> None: ...

class TestResp(_message.Message):
    __slots__ = ["error", "ms", "full_report"]
    ERROR_FIELD_NUMBER: _ClassVar[int]
    MS_FIELD_NUMBER: _ClassVar[int]
    FULL_REPORT_FIELD_NUMBER: _ClassVar[int]
    error: str
    ms: int
    full_report: str
    def __init__(self, error: _Optional[str] = ..., ms: _Optional[int] = ..., full_report: _Optional[str] = ...) -> None: ...

class QueryStatsReq(_message.Message):
    __slots__ = ["tag", "direct"]
    TAG_FIELD_NUMBER: _ClassVar[int]
    DIRECT_FIELD_NUMBER: _ClassVar[int]
    tag: str
    direct: str
    def __init__(self, tag: _Optional[str] = ..., direct: _Optional[str] = ...) -> None: ...

class QueryStatsResp(_message.Message):
    __slots__ = ["traffic"]
    TRAFFIC_FIELD_NUMBER: _ClassVar[int]
    traffic: int
    def __init__(self, traffic: _Optional[int] = ...) -> None: ...

class UpdateReq(_message.Message):
    __slots__ = ["action", "check_pre_release"]
    ACTION_FIELD_NUMBER: _ClassVar[int]
    CHECK_PRE_RELEASE_FIELD_NUMBER: _ClassVar[int]
    action: UpdateAction
    check_pre_release: bool
    def __init__(self, action: _Optional[_Union[UpdateAction, str]] = ..., check_pre_release: bool = ...) -> None: ...

class UpdateResp(_message.Message):
    __slots__ = ["error", "assets_name", "download_url", "release_url", "release_note", "is_pre_release"]
    ERROR_FIELD_NUMBER: _ClassVar[int]
    ASSETS_NAME_FIELD_NUMBER: _ClassVar[int]
    DOWNLOAD_URL_FIELD_NUMBER: _ClassVar[int]
    RELEASE_URL_FIELD_NUMBER: _ClassVar[int]
    RELEASE_NOTE_FIELD_NUMBER: _ClassVar[int]
    IS_PRE_RELEASE_FIELD_NUMBER: _ClassVar[int]
    error: str
    assets_name: str
    download_url: str
    release_url: str
    release_note: str
    is_pre_release: bool
    def __init__(self, error: _Optional[str] = ..., assets_name: _Optional[str] = ..., download_url: _Optional[str] = ..., release_url: _Optional[str] = ..., release_note: _Optional[str] = ..., is_pre_release: bool = ...) -> None: ...

class ListConnectionsResp(_message.Message):
    __slots__ = ["nekoray_connections_json"]
    NEKORAY_CONNECTIONS_JSON_FIELD_NUMBER: _ClassVar[int]
    nekoray_connections_json: str
    def __init__(self, nekoray_connections_json: _Optional[str] = ...) -> None: ...
