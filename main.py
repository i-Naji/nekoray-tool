from ctypes.util import find_library
from ctypes import *
import collections
import threading
import grpc
import requests
import json
from time import sleep, perf_counter
from concurrent.futures import ThreadPoolExecutor

from libcore_pb2_grpc import LibcoreServiceStub
from libcore_pb2 import *


# load shared library
path = "./build/libnekoray.so"
lib = CDLL(path)

_setup = lib.setup
_setup.argtypes = [c_char_p]
_setup.restype = None
_setup("/home/workshop/projects/go/proxy_gather/nekoray/deployment/linux64/nekobox_core".encode())
# _s = threading.Thread(target=_setup, args=("/home/workshop/projects/go/proxy_gather/nekoray/deployment/linux64/nekobox_core".encode('utf-8'),))
# _s.start()
# _s.join()

_url_test = lib.url_test
_url_test.argtypes = [c_char_p, c_char_p, c_int32]
_url_test.restype = c_char_p

_get_proxies = lib.get_proxies
_get_proxies.argtypes = [c_char_p]
_get_proxies.restype = c_char_p


_get_test_proxies = lib.get_test_proxies
_get_test_proxies.argtypes = [c_char_p]
_get_test_proxies.restype = c_char_p


link = "https://raw.githubusercontent.com/yebekhe/TelegramV2rayCollector/main/sub/normal/mix"


def get_test_proxies(content: str) -> dict:
    content = content.rstrip().strip()
    if content.startswith('http://') or content.startswith('https://'):
        res = requests.get(content)
        content = str(res.content, encoding='utf-8').encode()
    else:
        content = content.encode()

    bytes_proxies = _get_test_proxies(content).decode('utf-8')
    
    try:
        proxies = json.loads(bytes_proxies)
    except ValueError as e:
        print(e)
        proxies = {}
    return proxies



good = 0
# pp = get_test_proxies(link)
# print(len(pp))
# for addr, p in pp.items():
#     print(len(p))
# exit()

# print(len(pp))
class _GenericClientInterceptor(
    grpc.UnaryUnaryClientInterceptor,
    grpc.UnaryStreamClientInterceptor,
    grpc.StreamUnaryClientInterceptor,
    grpc.StreamStreamClientInterceptor,
):
    def __init__(self, interceptor_function):
        self._fn = interceptor_function

    def intercept_unary_unary(self, continuation, client_call_details, request):
        new_details, new_request_iterator, postprocess = self._fn(
            client_call_details, iter((request,)), False, False
        )
        response = continuation(new_details, next(new_request_iterator))
        return postprocess(response) if postprocess else response

    def intercept_unary_stream(
        self, continuation, client_call_details, request
    ):
        new_details, new_request_iterator, postprocess = self._fn(
            client_call_details, iter((request,)), False, True
        )
        response_it = continuation(new_details, next(new_request_iterator))
        return postprocess(response_it) if postprocess else response_it

    def intercept_stream_unary(
        self, continuation, client_call_details, request_iterator
    ):
        new_details, new_request_iterator, postprocess = self._fn(
            client_call_details, request_iterator, True, False
        )
        response = continuation(new_details, new_request_iterator)
        return postprocess(response) if postprocess else response

    def intercept_stream_stream(
        self, continuation, client_call_details, request_iterator
    ):
        new_details, new_request_iterator, postprocess = self._fn(
            client_call_details, request_iterator, True, True
        )
        response_it = continuation(new_details, new_request_iterator)
        return postprocess(response_it) if postprocess else response_it


def create(intercept_call):
    return _GenericClientInterceptor(intercept_call)


class _ClientCallDetails(
    collections.namedtuple(
        "_ClientCallDetails", ("method", "timeout", "metadata", "credentials")
    ),
    grpc.ClientCallDetails,
):
    pass


def header_adder_interceptor(header, value):
    def intercept_call(
        client_call_details,
        request_iterator,
        request_streaming,
        response_streaming,
    ):
        metadata = []
        if client_call_details.metadata is not None:
            metadata = list(client_call_details.metadata)
        metadata.append(
            (
                header,
                value,
            )
        )
        client_call_details = _ClientCallDetails(
            client_call_details.method,
            client_call_details.timeout,
            metadata,
            client_call_details.credentials,
        )
        return client_call_details, request_iterator, None

    return create(intercept_call)

header_adder_interceptor = (
        header_adder_interceptor(
            "nekoray_auth", "123456789"
        )
    )

channel = grpc.insecure_channel('127.0.0.1:1552')
intercept_channel = grpc.intercept_channel(
            channel, header_adder_interceptor
        )

stub = LibcoreServiceStub(intercept_channel)




def divide_chunks(l, n): 
      
    # looping till length l 
    for i in range(0, len(l), n):  
        yield l[i:i + n] 
  
# How many elements each 
# list should have 
n = 5


def test(addr, config, num):
    global good
    x : TestResp = stub.Test(
        TestReq(
            mode=TestMode.UrlTest,
            timeout=3000,
            url="http://cp.cloudflare.com/",
            config=LoadConfigReq(core_config=json.dumps(config)),
            in_address=addr,
        )
    )
    
    if x.error:
        print(num, f'> {x.error}')
    else:
        print(num, '>', x.ms)
        good += 1

# i = 0
# for pl in divide_chunks(list(pp.items()), n):
#     tasks = []
#     for addr, config in pl:
#         i += 1
#         tasks.append(threading.Thread(target=test, args=(addr, config,i)))
#         tasks[-1].start()
#     for t in tasks:
#         t.join()


def get_proxies(content: str) -> list[str]:
    content = content.rstrip().strip()
    if content.startswith('http://') or content.startswith('https://'):
        res = requests.get(content)
        content = str(res.content, encoding='utf-8').encode()
    else:
        content = content.encode()

    bytes_proxies = _get_proxies(content).decode('utf-8')
    
    try:
        proxies = json.loads(bytes_proxies)
    except ValueError as e:
        print(e)
        proxies = []
    return proxies


pp = get_proxies(link)


def test2(proxy, num):
    global good
    x = _url_test(proxy.encode(), "http://cp.cloudflare.com/".encode(), 3000).decode()
    
    if 'Error' in x:
        print(num, 'E >', x)
    else:
        print(num, '>', x)
        good += 1
    
i = 0
for pl in divide_chunks(pp, n):
    tasks = []
    for p in pl:
        i += 1
        tasks.append(threading.Thread(target=test2, args=(p,i)))
        tasks[-1].start()
    for t in tasks:
        t.join()

print(good)