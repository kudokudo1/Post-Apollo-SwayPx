#include <assert.h>
#include <stdio.h>
#include "sway/input/post_apollo_autoscroll_topology.h"

static void test_nonvirtual_paths_are_only_candidates(void) {
    assert(pa_autoscroll_kernel_tree_candidate(
        "/sys/devices/pci0000:00/0000:00:14.0/usb1/1-1/"
        "1-1:1.0/0003:1234:5678.0001/input/input14/event4",
        "/dev/input/event4"));
    assert(pa_autoscroll_kernel_tree_candidate(
        "/sys/devices/platform/i8042/serio1/input/input2/event12",
        "/dev/input/event12"));
}
static void test_virtual_and_inconsistent_paths_are_rejected(void) {
    const char *good =
        "/sys/devices/platform/i8042/serio1/input/input2/event12";
    assert(!pa_autoscroll_kernel_tree_candidate(NULL, "/dev/input/event12"));
    assert(!pa_autoscroll_kernel_tree_candidate(good, NULL));
    assert(!pa_autoscroll_kernel_tree_candidate(
        "/sys/devices/virtual/input/input8/event8", "/dev/input/event8"));
    assert(!pa_autoscroll_kernel_tree_candidate(
        "/sys/devices/virtual/misc/uhid/0005:1234:5678.1/input/input3/event3",
        "/dev/input/event3"));
    assert(!pa_autoscroll_kernel_tree_candidate(
        "/sys/devices/platform/uinput/input/input2/event12",
        "/dev/input/event12"));
    assert(!pa_autoscroll_kernel_tree_candidate(
        "/sys/devices/../devices/platform/i8042/serio1/input/input2/event12",
        "/dev/input/event12"));
    assert(!pa_autoscroll_kernel_tree_candidate(
        "/sys/devices//platform/i8042/serio1/input/input2/event12",
        "/dev/input/event12"));
    assert(!pa_autoscroll_kernel_tree_candidate(
        "/sys/devices/platform/i8042/serio1/input/input2/event12",
        "/dev/input/event13"));
    assert(!pa_autoscroll_kernel_tree_candidate(
        "/sys/devices/platform/i8042/serio1/input/input2/event12",
        "/dev/input/event12-extra"));
    assert(!pa_autoscroll_kernel_tree_candidate(
        "/sys/devices/platform/i8042/serio1/event12",
        "/dev/input/event12"));
    assert(!pa_autoscroll_kernel_tree_candidate(
        "/sys/devices/platform/i8042/serio1/input/inputX/event12",
        "/dev/input/event12"));
    assert(!pa_autoscroll_kernel_tree_candidate(
        "/sys/devices/platform/i8042/serio1/input/input2/eventX",
        "/dev/input/eventX"));
    assert(!pa_autoscroll_kernel_tree_candidate(
        "/sys/devices/platform/i8042/serio1/input/input2/event12",
        "relative/event12"));
}

int main(void) {
    test_nonvirtual_paths_are_only_candidates();
    test_virtual_and_inconsistent_paths_are_rejected();
    puts("PA_AUTOSCROLL_TOPOLOGY_NEGATIVE_FILTER_PASS "
         "genuine_hardware_proof=NONE");
    return 0;
}
