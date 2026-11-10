#define _GNU_SOURCE
#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "sway/stage_director_company_bundle_inventory.h"
#include "sway/stage_director_company_digest.h"
#include "sway/stage_director_company_launch_request.h"
#include "sway/stage_director_company_pre_spawn_custody.h"

static void file_create(const char *path, const char *content, mode_t mode) {
    const int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, mode);
    assert(fd >= 0);
    assert(write(fd, content, strlen(content)) == (ssize_t)strlen(content));
    assert(close(fd) == 0);
    assert(chmod(path, mode) == 0);
}
static void file_replace(const char *path, const char *content) {
    const int fd = open(path, O_WRONLY | O_TRUNC | O_CLOEXEC);
    assert(fd >= 0);
    assert(write(fd, content, strlen(content)) == (ssize_t)strlen(content));
    assert(close(fd) == 0);
}
static void hash_file(const char *path, char hex[65]) {
    const int fd = open(path, O_RDONLY | O_CLOEXEC);
    assert(fd >= 0);
    assert(stage_director_company_sha256_fd(fd, hex));
    assert(close(fd) == 0);
}
static void no_authority(struct stage_director_company_pre_spawn_receipt r) {
    assert(!r.dependencies_closed && !r.bundle_immutable);
    assert(!r.executable_approved && !r.operator_authenticated);
    assert(!r.launch_authenticated && !r.can_spawn);
    assert(!r.can_apply_scene && !r.can_accept_input);
    assert(!r.can_focus_keyboard && !r.can_show_popup);
}
static struct stage_director_company_launch_request_identity native(void) {
    return (struct stage_director_company_launch_request_identity) {
        .selection_epoch=99, .intent_sequence=31, .host_revision=5,
        .native_host_layer_ref=100, .native_host_surface_ref=101,
        .native_host_client_ref=102, .native_output_ref=103,
        .mode=1, .map_in_host={.x=7,.y=12,.width=1493,.height=788},
    };
}
int main(void) {
    char bundle[] = "/tmp/company-m32t-custody-XXXXXX";
    assert(mkdtemp(bundle) != NULL);
    const char *dirs[]={"bin","lib","plugins"};
    const char *names[]={"company-vector-probe","libQt6Sample.so",
                         "libqwaylandSample.so"};
    const char *contents[]={"native-exec-fixture","library-fixture","plugin-fixture"};
    char paths[3][512], name[512], hashes[3][65];
    for (int i=0;i<3;++i) {
        snprintf(name,sizeof(name),"%s/%s",bundle,dirs[i]);
        assert(mkdir(name,0700)==0);
        snprintf(paths[i],sizeof(paths[i]),"%s/%s/%s",bundle,dirs[i],names[i]);
        file_create(paths[i],contents[i],i==0?0700:0600);
        hash_file(paths[i],hashes[i]);
    }
    const int root=open(bundle,O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
    assert(root>=0);
    struct stage_director_company_bundle_entry manifest[]={
        {SD_COMPANY_BUNDLE_EXECUTABLE,names[0],hashes[0]},
        {SD_COMPANY_BUNDLE_LIBRARY,names[1],hashes[1]},
        {SD_COMPANY_BUNDLE_QT_PLUGIN,names[2],hashes[2]},
    };
    struct stage_director_company_launch_request request={0};
    stage_director_company_launch_request_init(&request);
    struct stage_director_company_launch_request_identity id=native();
    struct stage_director_company_pre_spawn_custody c={0};
    stage_director_company_pre_spawn_init(&c);
    uint64_t generation=0, now=UINT64_C(1000000000);
    assert(stage_director_company_launch_request_reserve(
        &request,&id,now,SD_COMPANY_LAUNCH_REQUEST_MAX_NS,&generation));
    assert(generation==1);
    assert(stage_director_company_pre_spawn_capture(
        &c,&request,generation,&id,root,getuid(),manifest,3,now+10));
    assert(c.held && c.bundle_fd>=0 && c.executable_fd>=0);
    assert(c.last_generation==1 && c.generation==generation);
    const int held_exe=c.executable_fd;
    assert(fcntl(held_exe,F_GETFD)&FD_CLOEXEC);
    assert(!stage_director_company_pre_spawn_capture(
        &c,&request,generation,&id,root,getuid(),manifest,3,now+20));
    struct stage_director_company_pre_spawn_receipt p=
        stage_director_company_pre_spawn_probe(
            &c,&request,generation,&id,now+100);
    assert(p.state==SD_COMPANY_PRE_SPAWN_LISTED_BYTES_CANDIDATE_ONLY);
    assert(p.request_and_listed_bytes_candidate);
    no_authority(p);
    p=stage_director_company_pre_spawn_probe(&c,&request,500,&id,now+101);
    assert(p.state==SD_COMPANY_PRE_SPAWN_STALE_GENERATION && c.held);
    no_authority(p);
    assert(!stage_director_company_pre_spawn_revoke(&c,500) && c.held);
    struct stage_director_company_launch_request_identity wrong=id;
    wrong.native_output_ref=999;
    p=stage_director_company_pre_spawn_probe(
        &c,&request,generation,&wrong,now+102);
    assert(p.state==SD_COMPANY_PRE_SPAWN_REQUEST_CHANGED && !c.held);
    assert(fcntl(held_exe,F_GETFD)==-1);
    no_authority(p);
    /* M32r's own mismatch already revoked request. New generation only. */
    now=UINT64_C(2000000000);
    assert(stage_director_company_launch_request_reserve(
        &request,&id,now,SD_COMPANY_LAUNCH_REQUEST_MAX_NS,&generation));
    assert(generation==2);
    assert(stage_director_company_pre_spawn_capture(
        &c,&request,generation,&id,root,getuid(),manifest,3,now+10));
    /* Changing even a listed Qt library between capture and probe revokes. */
    file_replace(paths[1],"library-fixture-replaced");
    p=stage_director_company_pre_spawn_probe(
        &c,&request,generation,&id,now+20);
    assert(p.state==SD_COMPANY_PRE_SPAWN_IMAGE_CHANGED && !c.held);
    no_authority(p);
    file_replace(paths[1],contents[1]);
    assert(stage_director_company_launch_request_revoke(&request));

    now=UINT64_C(3000000000);
    assert(stage_director_company_launch_request_reserve(
        &request,&id,now,SD_COMPANY_LAUNCH_REQUEST_MAX_NS,&generation));
    assert(generation==3);
    assert(stage_director_company_pre_spawn_capture(
        &c,&request,generation,&id,root,getuid(),manifest,3,now+10));
    /* The file in bin was swapped after a pinned descriptor was obtained. */
    assert(unlink(paths[0])==0);
    file_create(paths[0],contents[0],0700);
    p=stage_director_company_pre_spawn_probe(
        &c,&request,generation,&id,now+20);
    assert(p.state==SD_COMPANY_PRE_SPAWN_IMAGE_CHANGED && !c.held);
    assert(stage_director_company_launch_request_revoke(&request));

    now=UINT64_C(4000000000);
    assert(stage_director_company_launch_request_reserve(
        &request,&id,now,SD_COMPANY_LAUNCH_REQUEST_MAX_NS,&generation));
    assert(generation==4);
    assert(stage_director_company_pre_spawn_capture(
        &c,&request,generation,&id,root,getuid(),manifest,3,now+10));
    p=stage_director_company_pre_spawn_probe(
        &c,&request,generation,&id,now+SD_COMPANY_PRE_SPAWN_MAX_NS+10);
    assert(p.state==SD_COMPANY_PRE_SPAWN_EXPIRED && !c.held);
    no_authority(p);
    assert(stage_director_company_launch_request_revoke(&request));

    now=UINT64_C(5000000000);
    assert(stage_director_company_launch_request_reserve(
        &request,&id,now,SD_COMPANY_LAUNCH_REQUEST_MAX_NS,&generation));
    assert(generation==5);
    assert(stage_director_company_pre_spawn_capture(
        &c,&request,generation,&id,root,getuid(),manifest,3,now+10));
    assert(stage_director_company_launch_request_revoke(&request));
    p=stage_director_company_pre_spawn_probe(
        &c,&request,generation,&id,now+20);
    assert(p.state==SD_COMPANY_PRE_SPAWN_REQUEST_CHANGED && !c.held);
    no_authority(p);
    now=UINT64_C(6000000000);
    assert(stage_director_company_launch_request_reserve(
        &request,&id,now,SD_COMPANY_LAUNCH_REQUEST_MAX_NS,&generation));
    assert(generation==6);
    assert(stage_director_company_pre_spawn_capture(
        &c,&request,generation,&id,root,getuid(),manifest,3,now+10));
    assert(stage_director_company_pre_spawn_revoke(&c,generation));
    assert(!stage_director_company_pre_spawn_revoke(&c,generation));
    assert(!stage_director_company_pre_spawn_capture(
        &c,&request,generation,&id,root,getuid(),manifest,3,now+20));
    assert(stage_director_company_launch_request_revoke(&request));
    close(root);
    for (int i=0;i<3;++i) {
        assert(unlink(paths[i])==0);
        snprintf(name,sizeof(name),"%s/%s",bundle,dirs[i]);
        assert(rmdir(name)==0);
    }
    assert(rmdir(bundle)==0);
    puts("SD_COMPANY_M32T request_bound=PASS pinned_fd=READONLY "
         "replay=DENIED library_mutation=REVOKED executable_swap=REVOKED "
         "expiry=REVOKED actual_spawn=OFF operator=OFF scene=OFF PASS");
    return 0;
}
