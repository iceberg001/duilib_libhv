#include "libhv/hv.h"
#include "libhv/http_client.h"
#include "libhv/requests.h"
#include "libhv/TcpClient.h"
#include "libhv/UdpClient.h"
#include "libhv/HttpMessage.h"
#include "libhv/HttpService.h"
void TestHttpRequest() {
    HttpRequest req;
    req.method = HTTP_GET;
    req.url = "http://127.0.0.1:8080/ping";
    requests::Request r(new HttpRequest);
    r->method = HTTP_GET;
}
