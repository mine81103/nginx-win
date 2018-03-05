#include <string>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")


extern "C" void *(*pcre_malloc)(size_t) = nullptr;
extern "C" void(*pcre_free)(void *) = nullptr;


static std::vector<std::string> get_all_adapter_ips(bool wifi_only);

static thread_local SOCKADDR_IN _adapter_addr{};


static std::string get_preferred_adapter_ip(const std::string& adapter_ip_pattern, bool wifi_only)
{
    std::string adapter_ip_str;
    std::string prefix = adapter_ip_pattern;
    if (adapter_ip_pattern.back() == '*') {
        prefix = adapter_ip_pattern.substr(0, adapter_ip_pattern.length() - 1);
    }

    for (const auto& ip : get_all_adapter_ips(wifi_only)) {
        if (ip.find(prefix) == 0) {
            adapter_ip_str = ip;
            break;
        }
    }

    return adapter_ip_str;
}

extern "C"
int get_preferred_adapter_addr(unsigned wifi_only, const char *adapter_ip_pattern, int adapter_ip_pattern_len,
	struct sockaddr **adapter_addr)
{
    if ((wifi_only == 0) && (adapter_ip_pattern == nullptr || adapter_ip_pattern_len <= 0)) {
        return 0;
    }

    // upstream_ip => ip string
    std::string adapter_ip_pattern_str(adapter_ip_pattern, adapter_ip_pattern_len);
    std::string adapter_ip = get_preferred_adapter_ip(adapter_ip_pattern_str, wifi_only != 0);
    if (adapter_ip.empty()) {
        return 0;
    }

    int r = InetPton(AF_INET, adapter_ip.c_str(), &_adapter_addr.sin_addr);
    if (1 == r) {
        _adapter_addr.sin_family = AF_INET;
        _adapter_addr.sin_port = 0;

        if (adapter_addr) {
            *adapter_addr = (struct sockaddr *)&_adapter_addr;
        }

        return sizeof(_adapter_addr);
    }

    return 0;
}


#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

static std::vector<std::string> get_all_adapter_ips(bool wifi_only)
{
    std::vector<std::string> adapter_ips;
    PIP_ADAPTER_INFO pAdapterInfo = NULL;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    {
        IP_ADAPTER_INFO dummy;
        if (GetAdaptersInfo(&dummy, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
            pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(ulOutBufLen);
            if (pAdapterInfo == NULL) {
                printf("Error allocating memory needed to call GetAdaptersinfo\n");
                return adapter_ips;
            }
        }
    }

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        while (pAdapter) {
            if (wifi_only) {
                std::string desc(pAdapter->Description);
                std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);

                if (std::string::npos != desc.find("wireless") ||
                    std::string::npos != desc.find("802.11") ||
                    std::string::npos != desc.find("wlan") ||
                    std::string::npos != desc.find("wi-fi"))
                {
                    adapter_ips.push_back(pAdapter->IpAddressList.IpAddress.String);
                    break;
                }
            }
            else {
                adapter_ips.push_back(pAdapter->IpAddressList.IpAddress.String);
            }

            pAdapter = pAdapter->Next;
        }
    }
    else {
        printf("GetAdaptersInfo failed with error: %d\n", dwRetVal);
    }

    if (pAdapterInfo) {
        FREE(pAdapterInfo);
    }

    return adapter_ips;
}
