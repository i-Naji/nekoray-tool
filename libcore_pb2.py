# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: libcore.proto
"""Generated protocol buffer code."""
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database
from google.protobuf.internal import builder as _builder
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()




DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\rlibcore.proto\x12\x07libcore\"\n\n\x08\x45mptyReq\"\x0b\n\tEmptyResp\"\x1a\n\tErrorResp\x12\r\n\x05\x65rror\x18\x01 \x01(\t\"a\n\rLoadConfigReq\x12\x13\n\x0b\x63ore_config\x18\x01 \x01(\t\x12\"\n\x1a\x65nable_nekoray_connections\x18\x02 \x01(\x08\x12\x17\n\x0fstats_outbounds\x18\x03 \x03(\t\"\xc9\x02\n\x07TestReq\x12\x1f\n\x04mode\x18\x01 \x01(\x0e\x32\x11.libcore.TestMode\x12\x0f\n\x07timeout\x18\x06 \x01(\x05\x12\x0f\n\x07\x61\x64\x64ress\x18\x02 \x01(\t\x12&\n\x06\x63onfig\x18\x03 \x01(\x0b\x32\x16.libcore.LoadConfigReq\x12\x0f\n\x07inbound\x18\x04 \x01(\t\x12\x0b\n\x03url\x18\x05 \x01(\t\x12\x12\n\nin_address\x18\x07 \x01(\t\x12\x14\n\x0c\x66ull_latency\x18\x08 \x01(\x08\x12\x12\n\nfull_speed\x18\t \x01(\x08\x12\x16\n\x0e\x66ull_speed_url\x18\r \x01(\t\x12\x1a\n\x12\x66ull_speed_timeout\x18\x0e \x01(\x05\x12\x13\n\x0b\x66ull_in_out\x18\n \x01(\x08\x12\x18\n\x10\x66ull_udp_latency\x18\x0c \x01(\x08\x12\x14\n\x08\x66ull_nat\x18\x0b \x01(\x08\x42\x02\x18\x01\":\n\x08TestResp\x12\r\n\x05\x65rror\x18\x01 \x01(\t\x12\n\n\x02ms\x18\x02 \x01(\x05\x12\x13\n\x0b\x66ull_report\x18\x03 \x01(\t\",\n\rQueryStatsReq\x12\x0b\n\x03tag\x18\x01 \x01(\t\x12\x0e\n\x06\x64irect\x18\x02 \x01(\t\"!\n\x0eQueryStatsResp\x12\x0f\n\x07traffic\x18\x01 \x01(\x03\"M\n\tUpdateReq\x12%\n\x06\x61\x63tion\x18\x01 \x01(\x0e\x32\x15.libcore.UpdateAction\x12\x19\n\x11\x63heck_pre_release\x18\x02 \x01(\x08\"\x89\x01\n\nUpdateResp\x12\r\n\x05\x65rror\x18\x01 \x01(\t\x12\x13\n\x0b\x61ssets_name\x18\x02 \x01(\t\x12\x14\n\x0c\x64ownload_url\x18\x03 \x01(\t\x12\x13\n\x0brelease_url\x18\x04 \x01(\t\x12\x14\n\x0crelease_note\x18\x05 \x01(\t\x12\x16\n\x0eis_pre_release\x18\x06 \x01(\x08\"7\n\x13ListConnectionsResp\x12 \n\x18nekoray_connections_json\x18\x01 \x01(\t*2\n\x08TestMode\x12\x0b\n\x07TcpPing\x10\x00\x12\x0b\n\x07UrlTest\x10\x01\x12\x0c\n\x08\x46ullTest\x10\x02*\'\n\x0cUpdateAction\x12\t\n\x05\x43heck\x10\x00\x12\x0c\n\x08\x44ownload\x10\x01\x32\x94\x03\n\x0eLibcoreService\x12/\n\x04\x45xit\x12\x11.libcore.EmptyReq\x1a\x12.libcore.EmptyResp\"\x00\x12\x33\n\x06Update\x12\x12.libcore.UpdateReq\x1a\x13.libcore.UpdateResp\"\x00\x12\x35\n\x05Start\x12\x16.libcore.LoadConfigReq\x1a\x12.libcore.ErrorResp\"\x00\x12/\n\x04Stop\x12\x11.libcore.EmptyReq\x1a\x12.libcore.ErrorResp\"\x00\x12-\n\x04Test\x12\x10.libcore.TestReq\x1a\x11.libcore.TestResp\"\x00\x12?\n\nQueryStats\x12\x16.libcore.QueryStatsReq\x1a\x17.libcore.QueryStatsResp\"\x00\x12\x44\n\x0fListConnections\x12\x11.libcore.EmptyReq\x1a\x1c.libcore.ListConnectionsResp\"\x00\x42\x11Z\x0fgrpc_server/genb\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'libcore_pb2', _globals)
if _descriptor._USE_C_DESCRIPTORS == False:
  DESCRIPTOR._options = None
  DESCRIPTOR._serialized_options = b'Z\017grpc_server/gen'
  _TESTREQ.fields_by_name['full_nat']._options = None
  _TESTREQ.fields_by_name['full_nat']._serialized_options = b'\030\001'
  _globals['_TESTMODE']._serialized_start=927
  _globals['_TESTMODE']._serialized_end=977
  _globals['_UPDATEACTION']._serialized_start=979
  _globals['_UPDATEACTION']._serialized_end=1018
  _globals['_EMPTYREQ']._serialized_start=26
  _globals['_EMPTYREQ']._serialized_end=36
  _globals['_EMPTYRESP']._serialized_start=38
  _globals['_EMPTYRESP']._serialized_end=49
  _globals['_ERRORRESP']._serialized_start=51
  _globals['_ERRORRESP']._serialized_end=77
  _globals['_LOADCONFIGREQ']._serialized_start=79
  _globals['_LOADCONFIGREQ']._serialized_end=176
  _globals['_TESTREQ']._serialized_start=179
  _globals['_TESTREQ']._serialized_end=508
  _globals['_TESTRESP']._serialized_start=510
  _globals['_TESTRESP']._serialized_end=568
  _globals['_QUERYSTATSREQ']._serialized_start=570
  _globals['_QUERYSTATSREQ']._serialized_end=614
  _globals['_QUERYSTATSRESP']._serialized_start=616
  _globals['_QUERYSTATSRESP']._serialized_end=649
  _globals['_UPDATEREQ']._serialized_start=651
  _globals['_UPDATEREQ']._serialized_end=728
  _globals['_UPDATERESP']._serialized_start=731
  _globals['_UPDATERESP']._serialized_end=868
  _globals['_LISTCONNECTIONSRESP']._serialized_start=870
  _globals['_LISTCONNECTIONSRESP']._serialized_end=925
  _globals['_LIBCORESERVICE']._serialized_start=1021
  _globals['_LIBCORESERVICE']._serialized_end=1425
# @@protoc_insertion_point(module_scope)