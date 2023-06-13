#include "brave/components/brave_wallet/browser/brave_wallet_utils.h"

#include <iostream>
#include <chrono>
#include <thread>

#include "base/test/bind.h"
#include "base/test/scoped_run_loop_timeout.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/android/tab_model/tab_model.h"
#include "chrome/browser/ui/android/tab_model/tab_model_list.h"
#include "chrome/test/base/android/android_browser_test.h"
#include "chrome/test/base/chrome_test_utils.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "chrome/browser/net/secure_dns_config.h"
#include "chrome/browser/net/system_network_context_manager.h"
#include "content/public/test/test_navigation_observer.h"
#include "content/public/test/content_mock_cert_verifier.h"
#include "net/dns/mock_host_resolver.h"
#include "net/test/embedded_test_server/http_request.h"
#include "net/test/embedded_test_server/http_response.h"
#include "brave/components/constants/brave_paths.h"
#include "base/path_service.h"
#include "brave/components/brave_search/browser/brave_search_fallback_host.h"
#include "components/search_engines/template_url_service.h"
#include "chrome/browser/search_engines/template_url_service_factory.h"
#include "base/strings/string_util.h"
//#include "chrome/browser/ui/browser.h"
//#include "base/files/file_path.h"
#include "components/google/core/common/google_switches.h"


class AndroidBrowserTestSwap : public PlatformBrowserTest {
public:

 void SetUpOnMainThread() override {
    LOG(INFO) << "SetUpOnMainThread Start";
    PlatformBrowserTest::SetUpOnMainThread();
 }

  content::WebContents* GetActiveWebContents() {
    return chrome_test_utils::GetActiveWebContents(this);
  }
};

IN_PROC_BROWSER_TEST_F(AndroidBrowserTestSwap, TestSwapPageAppearing) {
 EXPECT_TRUE(content::NavigateToURL(GetActiveWebContents(),
                                       GURL("brave://wallet/swap")));
}