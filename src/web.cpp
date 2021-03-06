#include "web.h"
#include "persistency.h"

bool addNextCard = false;
AsyncWebServer server(80);

const char htmlIndex[] PROGMEM = R"rawliteral(<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width, initial-scale=1"><link rel="icon" href="data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAEAAAABACAMAAACdt4HsAAAAilBMVEUAAAAKlNYYms8YmM8Yms4Xmc8Yl8sXmc4Ymc8Xms8Xmc8Xk8wVldAYmM0Yms8ZmtAYmM8Ymc7////1+/2m2O3V7PYqodIcm9DN6fUzpdX7/f75/P7u+Py74vFfud4+qtYkntHg8fmy3e98xePn9ftpveBVtdxErdjD5POa0+qPzuiHyuaT0Oh0wuFw2/KGAAAAEXRSTlMABaQ6wa8f9baLWi0YlWpRSi680yMAAAJ2SURBVFjDpZfplqIwEIXRVrtdZrrvRcENUXHXfv/XGwmJBkOWOX4/OIe26zNWqooQWRn18aA/iv6P1hcMvlz/Ly+Sb1j4sIa3tNs2HPxpim/pdx8w8K9CMyCAyM4QQfRsK+gikHbUCML5bIr/xHsGNHPhHY+h5fj9ewpsefDnnxWZdy/gFsx99QCPYAe34Qc2NhSsPVUNOywZo5mg/psxrt8f1kZvwi3Y4EnKkpclDPBkTi5Q48jMSOqingXjcwhy8gBgxZv4IIa4qYDiRRBTsHzIAPxyD2QkCwDbJkHH2HXqgjGPwJTkGcCBFbVJC4sgIycoY2cAqj+eKND3pS7YUKKEKbAsL7zIHShkYWuCHjSoiGUvAzGBMaW91CTQGb1U0VaU3Z7V8sECJEAgIbmCSd9SRTeSWwAECf4ilptvYhFUiS/EdTPPSebwCEx2fLIDggSz6wEaC0rWqjYvbgFLtCyvKRkLeMcpYAWQrKasMREECvS4JSWQrXQKEix25xlKEjVT9er1C46QZMxB0UZHkje/QC746SsdV6Sqj5JGQfvV8JgMmZRgfpV7GgM4T/b1Uu5ZhzGUIOVjU7csqTVT5BLseBB3e9VqEmMeGFzlAtR3ryEomgQdWEnk7JipmK050u7AylJuKdUwyJ1T2YR8tnaqlcq0Lvjrf6ptWKCiHv/tfrQtmMCN83iXn8jQ4571uZ4EnlkHlngy9NhsFazcC7AbUjXLXPHOUz7pS8JQjzcLOvbloOt7U1h6cug7LPsP2+8Z1PKdefD9/gbJB4IYmq+K7790aVXt4Sfy0Q3ov5Y12j1hBo4g/4Z0jJdFD722tvJeZOMfahbgJcPqy1MAAAAASUVORK5CYII="><link rel="preconnect" href="https://fonts.gstatic.com"><link href="https://fonts.googleapis.com/css2?family=Roboto&display=swap" rel="stylesheet"><style>html{font-family:"Roboto"}.title{text-align:center}.card{box-shadow:0 1px 2px 0 rgba(0, 0, 0, .2), 0 2px 7px 0 rgba(0, 0, 0, .19);padding:1rem;transition:box-shadow .5s}.card:hover{box-shadow:0 8px 17px 0 rgba(0, 0, 0, .2), 0 6px 20px 0 rgba(0,0,0,.19)}#log_container,#key_container{display:flex;flex-wrap:wrap}#log_container>.card{min-width:128px;margin:4px 8px;flex-grow:1}#key_container>.card{min-width:320px;margin:8px 8px;flex-grow:1}#log_container>.card.accepted{background-color:#388e3c}#log_container>.card.denied{background-color:#c62828}</style> <script>logs=()=>{fetch("api/logs").then(response=>response.json()).then(data=>{document.getElementById("log_container").innerHTML="";for(const it of data.logs){const card=document.createElement("div");card.className=`card log ${it.accepted?"accepted":"denied"}`;card.appendChild(document.createTextNode(it.key));document.getElementById("log_container").appendChild(card);}});};keys=()=>{fetch("api/keys").then(response=>response.json()).then(data=>{document.getElementById("key_container").innerHTML="";for(const[i,it]of data.keys.entries()){const card=document.createElement("div");card.className="card key";const p=document.createElement("p");p.innerText=it.key;card.appendChild(p);const button=document.createElement("button");button.appendChild(document.createTextNode("Delete"));button.addEventListener("click",event=>{fetch(`/api/remove?pos=${i}`);});card.appendChild(button);document.getElementById("key_container").appendChild(card);}});};window.onload=()=>{console.debug("Window loaded.");logs();keys();};function addNextCard(){fetch("api/add").then(response=>response.json()).then(data=>{console.debug(data);});} setInterval(()=>{keys();logs();},1000);</script> </head><body><h1 class="title">Acces Control</h1><div id="key_container" class="container card"><h4 class="title">Keys</h4></div> <button onClick="addNextCard();" style="margin:16px auto;padding:16px;display:block;width:512px;"> Add the next card </button><div class="container card"><h4 class="title">Logs</h4><div id="log_container"></div></div></body></html>)rawliteral";

void setupAsyncWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Got request.");
        request->send_P(200, "text/html", htmlIndex, nullptr);
    });
    server.on("/api/keys", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "application/json", getKeysJson().c_str(), nullptr);
    });
    server.on("/api/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "application/json", getLogsJson().c_str(), nullptr);
    });
    server.on("/api/add", HTTP_GET, [](AsyncWebServerRequest *request) {
        addNextCard = true;
        request->send_P(200, "application/json", "{\"response\":true}", nullptr);
    });
    server.on("/api/remove", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!request->hasParam("pos")) {
            request->send_P(422, "application/json", "{\"response\":false}", nullptr);
            return;
        }

        if(removeKey(request->getParam("pos")->value().toInt())) {
            request->send_P(200, "application/json", "{\"response\":true}", nullptr);
        } else {
            request->send_P(200, "application/json", "{\"response\":false}", nullptr);
        }
    });

    server.begin();
}

void servRemove(WiFiClient &client, uint8_t pos) {
    beginJsonResponse(client);
    client.print("{\"response\":");
    client.print(removeKey(pos));
    client.println("}");
}