////
// Created by eddy on 18-2-2.
//
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <memory>
#include "ithread.h"
#include "upnp.h"
#include "upnpconfig.h"
//#include "DlnaDmrLogging.h"
//#include "DlnaDmrOutput.h"
#include "DlnaDmrUpnpRender.h"
#include "DlnaDmrUpnpTransport.h"
#include "DlnaDmrUpnpControl.h"
#include "DlnaDmrSdk.h"
//#include "DeviceIoWrapper.h"
//#include <TimeUtils.h>
#include "SysLog.h"
#define DLNA_DMR_SDK_TAG "DlnaDmrSdk"

namespace duerOSDcsApp {
namespace dueros_dlna {

struct upnp_device* device;

DlnaDmrSdk::DlnaDmrSdk() {
    _listen_port = 49494;

    _uuid = "DLNA-DMR-2018-03-24";
    _friendly_name = "DLNA-DuerOS";
}
/*
void DlnaDmrSdk::add_output_module(std::shared_ptr<IOutput> output_module) {
    Output::get_instance().add_output_module(output_module);
}
*/

void DlnaDmrSdk::set_uuid(std::string uuid) {
    _uuid = uuid;
}

std::string DlnaDmrSdk::get_uuid() {
    return _uuid;
}

void DlnaDmrSdk::set_friendly_name(std::string name) {
    _friendly_name = name;
}

std::string DlnaDmrSdk::get_friendly_name() {
    return _friendly_name;
}

void DlnaDmrSdk::set_listen_port(int port) {
    _listen_port = port;
}

int DlnaDmrSdk::get_listen_port() {
    return _listen_port;
}

static char* ip_address = nullptr;

int DlnaDmrSdk::start() {
    // dlna_dmr_sdk::add_output_module(std::shared_ptr<IOutput> output_module)在start()之前必须调用
    //assert(Output::get_instance().is_output_module_added());

    /* wwm - 2019/10/29 */
    //param_init();

    //Output::get_instance().output_init();

    struct upnp_device_descriptor* upnp_renderer;

    upnp_renderer = UpnpRender::get_instance().upnp_renderer_descriptor(get_friendly_name().c_str(),
                    get_uuid().c_str());

    if (upnp_renderer == nullptr) {
        LOG(ERROR, "upnp_render is null!");
        return -1;
    }

    if (get_listen_port() != 0 &&
            (get_listen_port() < 49152 || get_listen_port() > 65535)) {
        LOG(ERROR, "Parameter error: --port needs to be in "
                   "range [49152..65535] (but was set to %d)\n",
                   get_listen_port());
        return -1;
    }
    printf("######## wwm debug dlna friendly name: %s\n", upnp_renderer->friendly_name);
    device = UpnpDevice::get_instance()
             .upnp_device_init(upnp_renderer, ip_address, get_listen_port());

    if (device == nullptr) {
        LOG(ERROR, "Failed to initialize UPnP device");
        return -1;
    }

    UpnpTransport::get_instance().upnp_transport_init(device);
    UpnpControl::get_instance().upnp_control_init(device);

    UpnpTransport::get_instance().upnp_transport_register_variable_listener(log_variable_change,
            (void*) "transport");
    UpnpControl::get_instance().upnp_control_register_variable_listener(log_variable_change,
            (void*) "control");

    LOG(INFO, "Ready for rendering.\n");

    return 0;
}

int DlnaDmrSdk::stop() {
    param_reset();

    LOG(INFO, "Exiting.\n");
    UpnpDevice::get_instance().upnp_device_shutdown(device);
    return 0;
}

void DlnaDmrSdk::param_init() {
#if 0
    char m_uuid[32] = {0};
    unsigned int current_time = deviceCommonLib::deviceTools::currentTimeMs();

    set_friendly_name(application::DeviceIoWrapper::getInstance()->getlocalName());
    sprintf(m_uuid, "DUEROS-%s-%u", get_friendly_name().c_str(), current_time);
    set_uuid(std::string(m_uuid));
#endif
}

void DlnaDmrSdk::param_reset() {
    _listen_port = 49494;

    _uuid = "DLNA-DMR-2018-02-02";
    _friendly_name = "DLNA-DuerOS";

    ip_address = nullptr;
}

void DlnaDmrSdk::log_variable_change(void* userdata, int var_num,
                                     const char* variable_name,
                                     const char* old_value,
                                     const char* new_value) {
    const char* category = (const char*) userdata;
    int needs_newline = new_value[strlen(new_value) - 1] != '\n';

    LOG(INFO, "%s %s: old_value:%s new_value:%s%s", category, variable_name, old_value, new_value,
        needs_newline ? "\n" : "");
}
}
}
