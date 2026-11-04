#!/usr/bin/env python3
"""WATCH v2 wire compatibility and nonauthorizing candidate-only boundary."""
from pathlib import Path
import xml.etree.ElementTree as ET

xml = ET.parse("protocols/post-apollo-weather-watch-v1.xml").getroot()
assert xml.attrib["name"] == "post_apollo_weather_watch_v1"
interface, = xml.findall("interface")
assert interface.attrib["name"] == "post_apollo_weather_watch_v1"
assert interface.attrib["version"] == "2"
requests = {e.attrib["name"]: e for e in interface.findall("request")}
events = {e.attrib["name"]: e for e in interface.findall("event")}
# Wayland opcode order is ABI. Legacy v1 indexes MUST remain 0,1,2.
assert list(requests) == ["watch", "close", "destroy", "report_viewport"]
assert requests["watch"].get("since") is None
assert requests["close"].get("since") is None
assert requests["destroy"].get("since") is None
assert requests["destroy"].attrib["type"] == "destructor"
assert [(v.attrib["name"], v.attrib["type"]) for v in requests["watch"].findall("arg")] == [
    ("source_id", "string")]
report = requests["report_viewport"]
assert report.attrib["since"] == "2"
assert [(a.attrib["name"], a.attrib["type"]) for a in report.findall("arg")] == [
    ("x", "int"), ("y", "int"), ("width", "int"), ("height", "int")]
assert events["result"].get("since") is None
assert events["viewport_result"].attrib["since"] == "2"
assert [(a.attrib["name"], a.attrib["type"]) for a in
        events["viewport_result"].findall("arg")] == [
            ("status", "uint"), ("host_revision_hi", "uint"),
            ("host_revision_lo", "uint")]
enums = {e.attrib["name"]: e for e in interface.findall("enum")}
assert [(v.attrib["name"], v.attrib["value"]) for v in enums["status"]] == [
    ("denied", "0"), ("candidate_only", "1"),
    ("invalid_source", "2"), ("closed", "3")]
assert [(v.attrib["name"], v.attrib["value"]) for v in enums["viewport_status"]] == [
    ("denied", "0"), ("candidate_only", "1"), ("hidden", "2")]

protocol = Path("sway/stage_director_weather_watch_protocol.c").read_text()
stage = Path("sway/stage_director.c").read_text()
session = Path("sway/stage_director_news_host_session.c").read_text()
header = Path("include/sway/stage_director_news_host_session.h").read_text()
assert "version < 1 || version > 2" in protocol
assert "&post_apollo_weather_watch_v1_interface, 2, NULL, bind_watch" in protocol
assert "wl_resource_get_version(resource) < 2" in protocol
assert "client == wl_resource_get_client(resource)" in protocol
assert "request_epoch_current(client, resource)" in protocol
assert ".report_viewport = handle_report_viewport" in protocol
assert "stage_director_news_watch_report_viewport(" in protocol
assert "post_apollo_weather_watch_v1_send_viewport_result" in protocol
assert "stage_director_news_watch_probe_request(resource)" in stage
assert "weather_watch_lease.state != SD_NEWS_LEASE_HOST_CANDIDATE" in stage
assert "stage_director_weather_host_observe_native_peer(host)" in stage
assert "stage_director_news_host_session_update_geometry(" in stage
assert "stage_director_news_host_session_report_viewport(" in stage
assert "stage_director_news_host_session_viewport_current(" in stage
assert "stage_director_viewport_from_host_local(" in session
assert "clear_viewport_candidate(session);" in session
assert "stage_director_rect_contains(host.output, translated)" in session
assert "SD_NEWS_HOST_VIEWPORT_CANDIDATE_ONLY" in session
for denied in ("posix_spawn(", "fork(", "execve(", "system(",
               "wlr_scene_", "wlr_seat_", "kill("):
    assert denied not in session, denied
for flag in ("can_launch", "can_clip", "can_render",
             "can_accept_input", "can_show_popup"):
    assert flag in header, flag
assert "sd-ci" not in protocol.lower()  # This API is production, not a CI shortcut.
print("SD_WEATHER_WATCH_V2_BOUNDARY: v1_compatible=1 paired_peer=1 "
      "host_revision=1 geometry_candidate_only=1 authority=NONE PASS")
