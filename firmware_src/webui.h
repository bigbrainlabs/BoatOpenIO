// ============================================================
// BoatOpenIO – Web-UI & Captive Portal (v2.1 – responsive)
// ============================================================
#pragma once

#include <WebServer.h>
#include <DNSServer.h>

static DNSServer dnsServer;

static void startCaptivePortal() {
  dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
}

static void sendRedirect(const char* url) {
  webServer.sendHeader("Location", url, true);
  webServer.send(302, "text/plain", "");
}

static bool setupRequired() {
  return strlen(portal_pass) == 0;
}

static bool requireAuth() {
  if (!webServer.authenticate(portal_user, portal_pass)) {
    webServer.requestAuthentication(BASIC_AUTH, "BoatOpenIO");
    return false;
  }
  return true;
}

static bool isDefaultApPass() {
  return strcmp(ap_pass, AP_DEFAULT_PASS) == 0;
}

// ── ERSTEINRICHTUNG ──────────────────────────────────────────
void handleSetup() {
  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  webServer.send(200, "text/html", "");
  webServer.sendContent(
    "<!DOCTYPE html><html lang='de'><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>BoatOpenIO Setup</title>"
    "<style>"
    ":root{--bg:#0c0c0e;--c1:#141416;--c2:#1c1c1f;--bdr:#27272a;--acc:#6366f1;"
    "--ok:#22c55e;--warn:#f59e0b;--txt:#fafafa;--sub:#71717a;--r:10px}"
    "*{box-sizing:border-box;margin:0;padding:0}"
    "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;background:var(--bg);"
    "color:var(--txt);min-height:100vh;display:flex;align-items:center;justify-content:center;padding:20px}"
    ".wrap{width:100%;max-width:420px}"
    ".logo{text-align:center;margin-bottom:24px}"
    ".logo h1{font-size:1.4rem;font-weight:700;color:var(--txt);letter-spacing:-.01em}"
    ".logo p{color:var(--sub);font-size:.85rem;margin-top:5px}"
    ".card{background:var(--c1);border:1px solid var(--bdr);border-radius:var(--r);padding:24px}"
    ".warnb{background:rgba(245,158,11,.07);border:1px solid rgba(245,158,11,.2);padding:12px 14px;"
    "border-radius:8px;color:var(--warn);margin-bottom:20px;font-size:.84rem;line-height:1.55}"
    "h2{font-size:.68rem;font-weight:600;color:var(--sub);text-transform:uppercase;"
    "letter-spacing:.08em;margin:20px 0 10px}"
    "h2:first-of-type{margin-top:0}"
    "label{display:block;font-size:.8rem;color:var(--sub);margin:12px 0 4px;font-weight:500}"
    "input{background:var(--c2);color:var(--txt);border:1px solid var(--bdr);padding:10px 12px;"
    "width:100%;border-radius:7px;font-size:.9rem;outline:none;transition:border-color .15s}"
    "input:focus{border-color:var(--acc)}"
    ".sep{border:none;border-top:1px solid var(--bdr);margin:20px 0}"
    ".btn{padding:11px;margin-top:16px;width:100%;border:none;border-radius:7px;"
    "background:#22c55e;color:#000;font-size:.9rem;"
    "font-weight:600;cursor:pointer;transition:opacity .15s,transform .1s}"
    ".btn:hover{opacity:.8}.btn:active{transform:scale(.98)}"
    "</style></head><body>"
    "<div class='wrap'>"
    "<div class='logo'><h1>&#9875; BoatOpenIO</h1>"
    "<p>Universal Marine IO System &nbsp;&middot;&nbsp; v" FIRMWARE_VERSION "</p></div>"
    "<div class='card'>"
    "<div class='warnb'>&#9888;&ensp;First-time setup required. Set a portal password to secure the "
    "configuration interface.<br>Ersteinrichtung: bitte ein Portal-Passwort setzen.</div>"
    "<form method='POST' action='/dosetup'>"
    "<h2>Portal Access</h2>"
    "<label>User / Benutzer</label>"
    "<input name='pu' value='admin' required autocomplete='username'>"
    "<label>Password / Passwort</label>"
    "<input type='password' name='pp' required minlength='6' placeholder='min. 6 Zeichen' autocomplete='new-password'>"
    "<label>Confirm / Best&auml;tigen</label>"
    "<input type='password' name='pp2' required minlength='6' placeholder='Wiederholen' autocomplete='new-password'>"
    "<hr class='sep'>"
    "<h2>AP Credentials</h2>"
    "<label>AP Name (SSID)</label>"
    "<input name='as' value='"
  );
  webServer.sendContent(ap_ssid);
  webServer.sendContent(
    "' required>"
    "<label>AP Password (min. 8 Zeichen)</label>"
    "<input type='password' name='ap' value='"
  );
  webServer.sendContent(ap_pass);
  webServer.sendContent(
    "' required minlength='8' autocomplete='new-password'>"
    "<button class='btn' type='submit'>Setup abschlie&szlig;en &rarr;</button>"
    "</form></div></div></body></html>"
  );
}

void handleDoSetup() {
  String pu  = webServer.arg("pu");
  String pp  = webServer.arg("pp");
  String pp2 = webServer.arg("pp2");
  String as_ = webServer.arg("as");
  String ap_ = webServer.arg("ap");

  if (pp != pp2 || pp.length() < 6 || as_.length() < 1 || ap_.length() < 8) {
    webServer.send(400, "text/html",
      "<html><head><meta charset='UTF-8'>"
      "<meta http-equiv='refresh' content='2;url=/setup'></head>"
      "<body style='background:#0c0c0e;color:#ef4444;font-family:sans-serif;padding:20px'>"
      "<p>Invalid input / Ung&uuml;ltige Eingabe.</p></body></html>");
    return;
  }
  pu.toCharArray(portal_user, sizeof(portal_user));
  pp.toCharArray(portal_pass, sizeof(portal_pass));
  as_.toCharArray(ap_ssid,    sizeof(ap_ssid));
  ap_.toCharArray(ap_pass,    sizeof(ap_pass));
  saveNetConfig();
  webServer.send(200, "text/html",
    "<html><head><meta charset='UTF-8'></head>"
    "<body style='background:#0c0c0e;color:#22c55e;font-family:sans-serif;padding:20px'>"
    "<p>Setup complete. Restarting...</p></body></html>");
  delay(1000);
  ESP.restart();
}

// ── ROOT ─────────────────────────────────────────────────────
void handleRoot() {
  if (setupRequired()) { sendRedirect("/setup"); return; }
  if (!requireAuth())  return;

  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  webServer.send(200, "text/html", "");

  // ── HEAD + CSS ──────────────────────────────────────────────
  webServer.sendContent(
    "<!DOCTYPE html><html lang='de'><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>BoatOpenIO</title>"
    "<style>"
    ":root{--bg:#0c0c0e;--c1:#141416;--c2:#1c1c1f;--bdr:#27272a;"
    "--acc:#6366f1;--ok:#22c55e;--warn:#f59e0b;--err:#ef4444;"
    "--txt:#fafafa;--sub:#71717a;--r:10px}"
    "*{box-sizing:border-box}"
    "html{scroll-behavior:smooth}"
    "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',sans-serif;"
    "background:var(--bg);color:var(--txt);margin:0}"
    ".wrap{max-width:860px;margin:0 auto;padding:0 16px 60px}"
    ".hdr{position:sticky;top:0;z-index:100;background:rgba(12,12,14,.88);"
    "backdrop-filter:blur(16px);-webkit-backdrop-filter:blur(16px);"
    "border-bottom:1px solid var(--bdr);padding:13px 16px;"
    "display:flex;justify-content:space-between;align-items:center;margin:0 -16px}"
    "h1{font-size:1.05rem;font-weight:700;color:var(--txt);letter-spacing:-.01em;margin:0}"
    ".ver{font-size:.72rem;color:var(--sub);font-weight:400;margin-left:8px}"
    "h2{font-size:.68rem;font-weight:600;color:var(--sub);text-transform:uppercase;"
    "letter-spacing:.08em;margin:0 0 12px}"
    ".card{background:var(--c1);border:1px solid var(--bdr);border-radius:var(--r);"
    "padding:16px 20px;margin:8px 0}"
    "details.card{overflow:hidden}"
    "details.card>summary{cursor:pointer;list-style:none;display:flex;justify-content:space-between;"
    "align-items:center;margin:-16px -20px;padding:16px 20px;border-radius:var(--r);user-select:none;"
    "transition:background .15s}"
    "details.card>summary:hover{background:rgba(255,255,255,.03)}"
    "details.card>summary::-webkit-details-marker{display:none}"
    "details.card[open]>summary{border-radius:var(--r) var(--r) 0 0;"
    "border-bottom:1px solid var(--bdr);margin-bottom:14px}"
    ".chev{color:var(--sub);font-size:.95rem;transition:transform .2s;display:inline-block}"
    "details[open] .chev{transform:rotate(90deg)}"
    "label{display:block;font-size:.8rem;color:var(--sub);margin:12px 0 4px;font-weight:500}"
    "input,select{background:var(--c2);color:var(--txt);border:1px solid var(--bdr);"
    "padding:9px 12px;width:100%;border-radius:7px;font-size:.88rem;outline:none;"
    "transition:border-color .15s}"
    "input:focus,select:focus{border-color:var(--acc)}"
    "input[type=checkbox]{width:16px;height:16px;accent-color:var(--acc);cursor:pointer}"
    ".stg{display:grid;grid-template-columns:1fr 1fr;gap:6px}"
    ".sti{background:var(--c2);border:1px solid var(--bdr);border-radius:8px;padding:12px 14px}"
    ".sl{font-size:.65rem;color:var(--sub);font-weight:600;text-transform:uppercase;"
    "letter-spacing:.07em;margin-bottom:5px}"
    ".sv{font-size:.88rem;font-weight:500;display:flex;align-items:center;gap:6px;flex-wrap:wrap}"
    ".dot{width:6px;height:6px;border-radius:50%;display:inline-block;flex-shrink:0}"
    ".dok{background:var(--ok)}.derr{background:var(--err)}.dwarn{background:var(--warn)}"
    ".ok{color:var(--ok)}.err{color:var(--err)}.warn{color:var(--warn)}"
    ".achip{padding:2px 8px;border-radius:5px;font-size:.75rem;font-weight:600;font-family:monospace}"
    ".aok{background:rgba(34,197,94,.1);color:var(--ok);border:1px solid rgba(34,197,94,.2)}"
    ".aerr{background:rgba(239,68,68,.1);color:var(--err);border:1px solid rgba(239,68,68,.2)}"
    ".warnbox{background:rgba(245,158,11,.07);border:1px solid rgba(245,158,11,.2);"
    "padding:10px 14px;border-radius:8px;color:var(--warn);margin:8px 0;"
    "font-size:.84rem;display:flex;align-items:center;gap:10px}"
    "table{width:100%;border-collapse:collapse;font-size:.82rem}"
    "th{padding:8px 6px;text-align:left;font-size:.67rem;font-weight:600;color:var(--sub);"
    "text-transform:uppercase;letter-spacing:.05em;border-bottom:1px solid var(--bdr)}"
    "td{padding:5px 6px;border-bottom:1px solid rgba(255,255,255,.04)}"
    "tr:last-child td{border-bottom:none}"
    ".chwrap{overflow-x:auto;-webkit-overflow-scrolling:touch}"
    ".vg{display:grid;grid-template-columns:repeat(auto-fill,minmax(120px,1fr));gap:6px;margin-top:4px}"
    ".vc{background:var(--c2);border:1px solid var(--bdr);border-radius:8px;"
    "padding:14px 12px;text-align:center;transition:border-color .15s}"
    ".vc:hover{border-color:var(--acc)}"
    ".vn{font-size:.65rem;color:var(--sub);text-transform:uppercase;letter-spacing:.06em;"
    "margin-bottom:8px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}"
    ".vv{font-size:1.4rem;font-weight:600;color:var(--txt);line-height:1}"
    ".vu{font-size:.7rem;color:var(--sub);margin-top:4px}"
    "button{padding:9px 14px;margin:4px 0;border:none;border-radius:7px;font-size:.87rem;"
    "font-weight:500;cursor:pointer;width:100%;transition:opacity .15s,transform .1s}"
    "button:hover{opacity:.75}button:active{transform:scale(.98)}"
    ".bsave{background:#22c55e;color:#000}"
    ".breboot{background:#ef4444;color:#fff}"
    ".bcal{background:var(--acc);color:#fff}"
    ".bsec{background:var(--acc);color:#fff}"
    ".btest{background:#f59e0b;color:#000}"
    ".blive{background:#22c55e;color:#000}"
    ".blang{background:transparent;border:1px solid var(--bdr)!important;"
    "color:var(--sub);padding:5px 14px;width:auto;font-size:.8rem;border-radius:6px}"
    "#iw{display:flex;align-items:flex-start;gap:18px;margin:8px 0}"
    "#cal_status,#inv_status{font-size:.8rem;min-height:1.2em;color:var(--ok);margin-top:5px}"
    "@media(max-width:580px){"
    ".stg{grid-template-columns:1fr}"
    "#iw{flex-direction:column}"
    ".vg{grid-template-columns:repeat(auto-fill,minmax(100px,1fr))}"
    "}"
    "@media(max-width:400px){.vv{font-size:1.2rem}}"
    "</style>"
  );

  // ── JS ──────────────────────────────────────────────────────
  webServer.sendContent(
    "<script>"
    "var T={"
    "de:{"
    "h_wifi:'WiFi & MQTT',h_ch:'Kanalzuordnung',h_cal:'Kalibrierungs-Rechner',"
    "h_live:'Live-Werte',h_diag:'ADS Diagnose',"
    "h_imu:'IMU Kalibrierung',h_sec:'Sicherheit',h_act:'Aktionen',"
    "l_ssid:'WiFi SSID',l_wpass:'WiFi Passwort',l_mqtt:'MQTT Server',"
    "l_mport:'MQTT Port',l_muser:'MQTT User',l_mpass:'MQTT Passwort',"
    "save_net:'Speichern & Neustart',"
    "th_sensor:'Sensor',th_factor:'Faktor',th_offset:'Offset',th_unit:'Einheit',th_topic:'Topic',th_active:'Aktiv',"
    "save_ch:'Konfiguration speichern',"
    "sk_none:'Kein SK-Pfad bekannt für',"
    "cal_hint:'Faktor = Echter Wert / ADS-Spannung (A0)',"
    "l_real:'Echter Wert (Multimeter)',l_adsv:'ADS A0 Spannung [V]',l_factor:'Faktor',"
    "th_val:'Wert',loading:'Lade...',not_found:'nicht gefunden',"
    "imu_hint:'Boot auf ruhigem Wasser ausrichten, dann Null setzen.',"
    "imu_raw:'Rohwert',imu_corr:'Korrigiert',imu_off:'Offset',"
    "imu_bias:'Gyro-Bias (rad/s)',imu_btn:'Jetzt Null setzen',imu_saved:'Offset gespeichert',"
    "imu_inv_pitch:'Pitch invertieren',imu_inv_roll:'Roll invertieren',"
    "imu_inv_save:'Invertierung speichern',imu_inv_saved:'Gespeichert',"
    "sec_ap:'AP-Zugangsdaten',sec_portal:'Portal-Zugangsdaten',"
    "l_ap_ssid:'AP Name (SSID)',l_ap_pass:'AP Passwort (min. 8 Zeichen)',"
    "l_pu:'Portal Benutzer',l_pp:'Portal Passwort (min. 6 Zeichen)',"
    "l_pp2:'Passwort bestätigen',sec_save:'Speichern & Neustart',"
    "warn_defpass:'Standard-Passwort aktiv – bitte AP-Passwort ändern!',"
    "to_live:'→ LIVE (echte Sensoren)',to_test:'→ TEST (Fake-Daten)',"
    "restart:'Neustart',"
    "s_on:'verbunden',s_off:'getrennt',s_nowifi:'nicht verbunden',s_miss:'fehlt',"
    "s_uptime:'Uptime',s_mode:'Modus'"
    "},"
    "en:{"
    "h_wifi:'WiFi & MQTT',h_ch:'Channel Assignment',h_cal:'Calibration Calculator',"
    "h_live:'Live Values',h_diag:'ADS Diagnostics',"
    "h_imu:'IMU Calibration',h_sec:'Security',h_act:'Actions',"
    "l_ssid:'WiFi SSID',l_wpass:'WiFi Password',l_mqtt:'MQTT Server',"
    "l_mport:'MQTT Port',l_muser:'MQTT User',l_mpass:'MQTT Password',"
    "save_net:'Save & Restart',"
    "th_sensor:'Sensor',th_factor:'Factor',th_offset:'Offset',th_unit:'Unit',th_topic:'Topic',th_active:'Active',"
    "save_ch:'Save Configuration',"
    "sk_none:'No SK path known for',"
    "cal_hint:'Factor = Real Value / ADS Voltage (A0)',"
    "l_real:'Real Value (Multimeter)',l_adsv:'ADS A0 Voltage [V]',l_factor:'Factor',"
    "th_val:'Value',loading:'Loading...',not_found:'not found',"
    "imu_hint:'Level boat on calm water, then set zero.',"
    "imu_raw:'Raw',imu_corr:'Corrected',imu_off:'Offset',"
    "imu_bias:'Gyro Bias (rad/s)',imu_btn:'Set Zero Now',imu_saved:'Offset saved',"
    "imu_inv_pitch:'Invert Pitch',imu_inv_roll:'Invert Roll',"
    "imu_inv_save:'Save Inversion',imu_inv_saved:'Saved',"
    "sec_ap:'AP Credentials',sec_portal:'Portal Credentials',"
    "l_ap_ssid:'AP Name (SSID)',l_ap_pass:'AP Password (min. 8 chars)',"
    "l_pu:'Portal User',l_pp:'Portal Password (min. 6 chars)',"
    "l_pp2:'Confirm Password',sec_save:'Save & Restart',"
    "warn_defpass:'Default password active – please change the AP password!',"
    "to_live:'→ LIVE (real sensors)',to_test:'→ TEST (fake data)',"
    "restart:'Restart',"
    "s_on:'connected',s_off:'disconnected',s_nowifi:'not connected',s_miss:'missing',"
    "s_uptime:'Uptime',s_mode:'Mode'"
    "}"
    "};"
    "var curLang=localStorage.getItem('lang')||'de';"
    "function applyLang(l){"
    "var t=T[l];"
    "document.querySelectorAll('[data-i18n]').forEach(function(e){"
    "var k=e.getAttribute('data-i18n');"
    "if(t[k]!==undefined)e.textContent=t[k];"
    "});"
    "localStorage.setItem('lang',l);curLang=l;"
    "var b=document.getElementById('lbtn');if(b)b.textContent=l==='de'?'EN':'DE';"
    "}"
    "function toggleLang(){applyLang(curLang==='de'?'en':'de');}"
    "function skPath(s){"
    "s=s.toLowerCase();"
    "if(s.includes('batter')||s.includes('batteri'))return 'vessels/self/electrical/batteries/0/voltage';"
    "if(s.includes('oeldruck')||s.includes('oilpress')||s.includes('öldr'))return 'vessels/self/propulsion/engine/oilPressure';"
    "if(s.includes('oeltemp')||s.includes('oiltemp')||s.includes('ölt'))return 'vessels/self/propulsion/engine/oilTemperature';"
    "if(s.includes('kuehl')||s.includes('coolant'))return 'vessels/self/propulsion/engine/coolantTemperature';"
    "if(s.includes('tank'))return 'vessels/self/tanks/fuel/0/currentLevel';"
    "if(s.includes('drehzahl')||s.includes('rpm')||s.includes('rev'))return 'vessels/self/propulsion/engine/revolutions';"
    "if(s.includes('pitch'))return 'vessels/self/navigation/attitude/pitch';"
    "if(s.includes('roll'))return 'vessels/self/navigation/attitude/roll';"
    "if(s.includes('temp'))return 'vessels/self/propulsion/engine/temperature';"
    "if(s.includes('wind'))return 'vessels/self/environment/wind/speedApparent';"
    "if(s.includes('volt')||s.includes('span'))return 'vessels/self/electrical/batteries/0/voltage';"
    "return '';}"
    "function setSK(i){"
    "var sn=document.querySelector('input[name=\"s'+i+'\"]');"
    "var tn=document.querySelector('input[name=\"t'+i+'\"]');"
    "if(!sn||!tn)return;"
    "var p=skPath(sn.value);"
    "if(p){tn.value=p;}else{alert(T[curLang].sk_none+' \"'+sn.value+'\"');}}"
    "function updateHorizon(pitch,roll){"
    "var g=document.getElementById('imu-plane');if(!g)return;"
    "var py=Math.max(-38,Math.min(38,pitch*1.8));"
    "g.setAttribute('transform','rotate('+roll+') translate(0,'+py+')');"
    "}"
    "</script></head><body>"
  );

  // ── HEADER ───────────────────────────────────────────────────
  webServer.sendContent(
    "<div class='hdr'>"
    "<h1>&#9875; BoatOpenIO<span class='ver'>v" FIRMWARE_VERSION "</span></h1>"
    "<button class='blang' id='lbtn' onclick='toggleLang()'>EN</button>"
    "</div>"
    "<div class='wrap'>"
  );

  // ── DEFAULT-PASSWORT WARNUNG ──────────────────────────────────
  if (isDefaultApPass()) {
    webServer.sendContent(
      "<div class='warnbox'>&#9888;&ensp;"
      "<span data-i18n='warn_defpass'>Standard-Passwort aktiv &ndash; bitte AP-Passwort &auml;ndern!</span>"
      "</div>"
    );
  }

  // ── STATUS GRID ───────────────────────────────────────────────
  bool wifiUp = (WiFi.status() == WL_CONNECTED);
  webServer.sendContent("<div class='card'><h2>Status</h2><div class='stg'>");

  // AP
  webServer.sendContent(
    String("<div class='sti'><div class='sl'>Access Point</div>"
    "<div class='sv'><span class='dot dok'></span>") + ap_ssid + " &rarr; 192.168.4.1</div></div>"
  );

  // WiFi
  webServer.sendContent("<div class='sti'><div class='sl'>WiFi</div><div class='sv'>");
  if (wifiUp) {
    webServer.sendContent(
      String("<span class='dot dok'></span>") + WiFi.SSID() +
      " <small style='color:var(--sub)'>(" + WiFi.RSSI() + " dBm)</small>"
    );
  } else {
    webServer.sendContent("<span class='dot derr'></span><span class='warn' data-i18n='s_nowifi'>nicht verbunden</span>");
  }
  webServer.sendContent("</div></div>");

  // MQTT
  webServer.sendContent("<div class='sti'><div class='sl'>MQTT</div><div class='sv'>");
  if (mqtt.connected()) {
    webServer.sendContent(
      String("<span class='dot dok'></span><span class='ok' data-i18n='s_on'>verbunden</span>"
      " <small style='color:var(--sub)'>") + mqtt_server + "</small>"
    );
  } else {
    webServer.sendContent("<span class='dot derr'></span><span class='err' data-i18n='s_off'>getrennt</span>");
  }
  webServer.sendContent("</div></div>");

  // Mode
  webServer.sendContent("<div class='sti'><div class='sl' data-i18n='s_mode'>Modus</div><div class='sv'>");
  if (testMode) {
    webServer.sendContent("<span class='dot dwarn'></span><span style='color:var(--warn)'>TEST</span>");
  } else {
    webServer.sendContent("<span class='dot dok'></span><span class='ok'>LIVE</span>");
  }
  webServer.sendContent("</div></div>");

  // ADS1115 (single)
  webServer.sendContent("<div class='sti'><div class='sl'>ADS1115</div><div class='sv'>");
  webServer.sendContent(
    String("<span class='achip ") + (adsOK ? "aok" : "aerr") + "'>ADS1115 @ 0x48</span>"
  );
  webServer.sendContent("</div></div>");

  // MPU6050
  webServer.sendContent("<div class='sti'><div class='sl'>MPU6050</div><div class='sv'>");
  if (mpuOK) {
    webServer.sendContent("<span class='dot dok'></span><span class='ok'>OK</span>");
  } else {
    webServer.sendContent("<span class='dot derr'></span><span class='err' data-i18n='s_miss'>fehlt</span>");
  }
  webServer.sendContent("</div></div>");

  // Uptime
  webServer.sendContent(
    String("<div class='sti'><div class='sl' data-i18n='s_uptime'>Uptime</div>"
    "<div class='sv'>") + String(millis() / 1000) + " s</div></div>"
  );

  // Heap
  webServer.sendContent(
    String("<div class='sti'><div class='sl'>Heap</div>"
    "<div class='sv'>") + String(ESP.getFreeHeap() / 1024) + " KB free</div></div>"
  );

  webServer.sendContent(
    "<div class='sti'><div class='sl'>Firmware</div>"
    "<div class='sv'>v" FIRMWARE_VERSION "</div></div>"
  );

  webServer.sendContent("</div></div>"); // stg + card

  // ── WIFI & MQTT ───────────────────────────────────────────────
  webServer.sendContent(
    "<details class='card'>"
    "<summary><h2 data-i18n='h_wifi'>WiFi &amp; MQTT</h2><span class='chev'>&#8250;</span></summary>"
    "<form method='POST' action='/savenet'>"
  );
  webServer.sendContent(
    String("<label data-i18n='l_ssid'>WiFi SSID</label>"
    "<input name='ssid' value='") + wifi_ssid + "' autocomplete='off'>" +
    "<label data-i18n='l_wpass'>WiFi Passwort</label>"
    "<input type='password' name='wpass' placeholder='(unver&auml;ndert lassen)'>" +
    "<label data-i18n='l_mqtt'>MQTT Server</label>"
    "<input name='mqtt' value='" + mqtt_server + "'>" +
    "<label data-i18n='l_mport'>MQTT Port</label>"
    "<input name='mport' value='" + mqtt_port + "'>" +
    "<label data-i18n='l_muser'>MQTT User</label>"
    "<input name='muser' value='" + mqtt_user + "'>" +
    "<label data-i18n='l_mpass'>MQTT Passwort</label>"
    "<input type='password' name='mpass' placeholder='(unver&auml;ndert lassen)'>" +
    "<button class='bsave' type='submit' style='margin-top:12px' data-i18n='save_net'>Speichern &amp; Neustart</button>"
    "</form></details>"
  );

  // ── KANALZUORDNUNG ────────────────────────────────────────────
  webServer.sendContent(
    "<details class='card'>"
    "<summary><h2 data-i18n='h_ch'>Kanalzuordnung</h2><span class='chev'>&#8250;</span></summary>"
    "<p style='font-size:.78rem;color:var(--sub);margin-bottom:10px'>"
    "MUX &rarr; ADS1115 A0 &rarr; 0x48 &mdash; alle 16 Kan&auml;le &uuml;ber einen ADC.</p>"
    "<form method='POST' action='/save'>"
    "<div class='chwrap'>"
    "<table><tr>"
    "<th>#</th>"
    "<th data-i18n='th_sensor'>Sensor</th>"
    "<th data-i18n='th_factor'>Faktor</th>"
    "<th data-i18n='th_offset'>Offset</th>"
    "<th data-i18n='th_unit'>Einheit</th>"
    "<th data-i18n='th_topic'>Topic</th>"
    "<th data-i18n='th_active'>Aktiv</th>"
    "</tr>"
  );
  for (int i = 0; i < 16; i++) {
    String sensorVal = kanaele[i].sensor;
    String r = String("<tr><td style='color:var(--sub);font-size:.75rem;font-weight:600'>") + (i + 1) + "</td>";
    r += String("<td><input name='s") + i + "' value='" + sensorVal + "' style='min-width:110px'></td>";
    r += String("<td><input name='f") + i + "' value='" + String(kanaele[i].faktor, 4) + "' style='width:70px'></td>";
    r += String("<td><input name='o") + i + "' value='" + String(kanaele[i].offset, 4) + "' style='width:70px'></td>";
    r += String("<td><input name='e") + i + "' value='" + kanaele[i].einheit + "' style='width:48px'></td>";
    r += String("<td><div style='display:flex;gap:4px;align-items:center'>"
                "<input name='t") + i + "' value='" + kanaele[i].topic +
                "' placeholder='boat/io/…' style='min-width:150px'>"
                "<button type='button' onclick='setSK(" + i + ")'"
                " style='width:auto;padding:4px 8px;font-size:.72rem;background:#27272a;"
                "color:#a1a1aa;border:1px solid #3f3f46;border-radius:6px;"
                "white-space:nowrap;cursor:pointer'>SK</button>"
                "</div></td>";
    r += String("<td style='text-align:center'><input type='checkbox' name='c") + i + "'" + (kanaele[i].aktiv ? " checked" : "") + "></td></tr>";
    webServer.sendContent(r);
  }
  webServer.sendContent(
    "</table></div>"
    "<button class='bsave' type='submit' style='margin-top:12px' data-i18n='save_ch'>Konfiguration speichern</button>"
    "</form></details>"
  );

  // ── KALIBRIERUNGS-RECHNER ─────────────────────────────────────
  webServer.sendContent(
    "<details class='card'>"
    "<summary><h2 data-i18n='h_cal'>Kalibrierungs-Rechner</h2><span class='chev'>&#8250;</span></summary>"
    "<p style='font-size:.82rem;color:var(--sub);margin-bottom:12px' data-i18n='cal_hint'>"
    "Faktor = Echter Wert / ADS-Spannung (A0)</p>"
    "<label data-i18n='l_real'>Echter Wert (Multimeter)</label>"
    "<input type='number' step='0.001' id='cr' oninput='cf()'>"
    "<label data-i18n='l_adsv'>ADS A0 Spannung [V]</label>"
    "<input type='number' step='0.001' id='cp' oninput='cf()'>"
    "<label data-i18n='l_factor'>Faktor</label>"
    "<input id='cres' readonly>"
    "<script>function cf(){"
    "var r=parseFloat(document.getElementById('cr').value),"
    "p=parseFloat(document.getElementById('cp').value);"
    "if(!isNaN(r)&&!isNaN(p)&&p!==0)"
    "document.getElementById('cres').value=(r/p).toFixed(4);}"
    "</script></details>"
  );

  // ── LIVE-WERTE ────────────────────────────────────────────────
  webServer.sendContent(
    "<div class='card'>"
    "<h2 data-i18n='h_live'>Live-Werte</h2>"
    "<div id='vals'><small style='color:var(--sub)' data-i18n='loading'>Lade...</small></div>"
    "<script>"
    "function reloadVals(){"
    "fetch('/api/values').then(function(r){return r.json();}).then(function(d){"
    "var s='<div class=vg>';"
    "for(var k in d){"
    "var v=d[k];"
    "var disp=(typeof v==='number')?parseFloat(v).toFixed(2):v;"
    "var unit=(k==='pitch'||k==='roll')?'&deg;':'';"
    "s+='<div class=vc>';"
    "s+='<div class=vn>'+k+'</div>';"
    "s+='<div class=vv>'+disp+'</div>';"
    "s+='<div class=vu>'+unit+'</div>';"
    "s+='</div>';}"
    "s+='</div>';"
    "document.getElementById('vals').innerHTML=s;"
    "}).catch(function(){});}"
    "reloadVals();setInterval(reloadVals,2000);"
    "</script></div>"
  );

  // ── ADS DIAGNOSE ──────────────────────────────────────────────
  webServer.sendContent(
    "<details class='card'>"
    "<summary><h2 data-i18n='h_diag'>ADS Diagnose</h2><span class='chev'>&#8250;</span></summary>"
    "<div id='raw'><small style='color:var(--sub)' data-i18n='loading'>Lade...</small></div>"
    "<script>"
    "function reloadRaw(){"
    "fetch('/api/raw').then(function(r){return r.json();}).then(function(d){"
    "var a=d.ads1;"
    "if(!a){document.getElementById('raw').innerHTML="
    "'<span style=\"color:var(--err)\">ADS1115 nicht gefunden</span>';return;}"
    "var s='<table><tr><th>ADS</th><th>Addr</th><th>A0 (aktiv)</th><th>A1</th><th>A2</th><th>A3</th></tr>';"
    "s+='<tr>';"
    "s+='<td>'+(a.ok?'<span class=ok>ADS1115</span>':'<span class=err>ADS1115</span>')+'</td>';"
    "s+='<td>0x48</td>';"
    "if(a.ok){"
    "s+='<td style=\"color:var(--acc);font-weight:600\">'+a.a0.toFixed(3)+'V</td>';"
    "s+='<td style=\"color:var(--sub)\">'+a.a1.toFixed(3)+'V</td>';"
    "s+='<td style=\"color:var(--sub)\">'+a.a2.toFixed(3)+'V</td>';"
    "s+='<td style=\"color:var(--sub)\">'+a.a3.toFixed(3)+'V</td>';"
    "}else{"
    "s+='<td colspan=4 style=\"color:var(--err)\">nicht gefunden</td>';}"
    "s+='</tr></table>';"
    "document.getElementById('raw').innerHTML=s;"
    "}).catch(function(){});}"
    "reloadRaw();setInterval(reloadRaw,2000);"
    "</script></details>"
  );

  // ── IMU KALIBRIERUNG ──────────────────────────────────────────
  webServer.sendContent(
    "<div class='card'>"
    "<h2 data-i18n='h_imu'>IMU Kalibrierung</h2>"
    "<p style='font-size:.82rem;color:var(--sub);margin-bottom:12px' data-i18n='imu_hint'>"
    "Boot auf ruhigem Wasser ausrichten, dann Null setzen.</p>"
    "<div id='iw'>"
    "<svg viewBox='-50 -50 100 100' width='110' height='110' style='flex-shrink:0'>"
    "<defs><clipPath id='cc'><circle r='47'/></clipPath></defs>"
    "<g clip-path='url(#cc)'>"
    "<g id='imu-plane'>"
    "<rect x='-60' y='-80' width='120' height='80' fill='#1e3a8a' opacity='.7'/>"
    "<rect x='-60' y='0' width='120' height='80' fill='#064e3b' opacity='.7'/>"
    "<line x1='-55' y1='0' x2='55' y2='0' stroke='#06b6d4' stroke-width='2'/>"
    "</g></g>"
    "<circle r='47' fill='none' stroke='rgba(6,182,212,.4)' stroke-width='2.5'/>"
    "<line x1='-20' y1='0' x2='20' y2='0' stroke='#f59e0b' stroke-width='2.5' stroke-linecap='round'/>"
    "<line x1='0' y1='-8' x2='0' y2='8' stroke='#f59e0b' stroke-width='2' stroke-linecap='round'/>"
    "<circle r='2.5' fill='#f59e0b'/>"
    "</svg>"
    "<div id='imu_table' style='flex:1'><small style='color:var(--sub)' data-i18n='loading'>Lade...</small></div>"
    "</div>"
    "<button class='bcal' onclick='doCalibrate()' data-i18n='imu_btn'>Jetzt Null setzen</button>"
    "<div id='cal_status'></div>"
    "<div style='margin-top:14px;padding-top:12px;border-top:1px solid rgba(255,255,255,.07)'>"
    "<label style='display:flex;align-items:center;gap:8px;margin:6px 0;cursor:pointer;width:auto'>"
    "<input type='checkbox' id='pitch_inv'>"
    "<span data-i18n='imu_inv_pitch'>Pitch invertieren</span></label>"
    "<label style='display:flex;align-items:center;gap:8px;margin:6px 0;cursor:pointer;width:auto'>"
    "<input type='checkbox' id='roll_inv'>"
    "<span data-i18n='imu_inv_roll'>Roll invertieren</span></label>"
    "<button class='bsave' onclick='saveInvert()' style='margin-top:8px' data-i18n='imu_inv_save'>"
    "Invertierung speichern</button>"
    "<div id='inv_status'></div>"
    "</div>"
    "<script>"
    "var imuInvLoaded=false;"
    "function reloadIMU(){"
    "fetch('/api/imu').then(function(r){return r.json();}).then(function(d){"
    "var t=T[curLang];"
    "var s='<table>';"
    "s+='<tr><th></th><th>Pitch</th><th>Roll</th></tr>';"
    "s+='<tr><td>'+t.imu_raw+'</td>"
    "<td>'+d.pitch_raw.toFixed(2)+'&deg;</td>"
    "<td>'+d.roll_raw.toFixed(2)+'&deg;</td></tr>';"
    "s+='<tr><td>'+t.imu_corr+'</td>"
    "<td><b>'+d.pitch_corr.toFixed(2)+'&deg;</b></td>"
    "<td><b>'+d.roll_corr.toFixed(2)+'&deg;</b></td></tr>';"
    "s+='<tr><td>'+t.imu_off+'</td>"
    "<td style=\"color:var(--sub)\">'+d.pitch_offset.toFixed(2)+'&deg;</td>"
    "<td style=\"color:var(--sub)\">'+d.roll_offset.toFixed(2)+'&deg;</td></tr>';"
    "s+='</table>';"
    "s+='<small style=\"color:#4a5568;margin-top:6px;display:block\">"
    "'+t.imu_bias+': X='+d.bias_x.toFixed(4)+' Y='+d.bias_y.toFixed(4)+' Z='+d.bias_z.toFixed(4)+'</small>';"
    "document.getElementById('imu_table').innerHTML=s;"
    "updateHorizon(d.pitch_corr,d.roll_corr);"
    "if(!imuInvLoaded){"
    "var pi=document.getElementById('pitch_inv');"
    "var ri=document.getElementById('roll_inv');"
    "if(pi)pi.checked=!!d.pitch_invert;"
    "if(ri)ri.checked=!!d.roll_invert;"
    "imuInvLoaded=true;}"
    "}).catch(function(){});}"
    "function saveInvert(){"
    "var fd=new FormData();"
    "if(document.getElementById('pitch_inv').checked)fd.append('pi','1');"
    "if(document.getElementById('roll_inv').checked)fd.append('ri','1');"
    "fetch('/setinvert',{method:'POST',body:fd})"
    ".then(function(r){return r.json();})"
    ".then(function(d){"
    "if(d.ok){"
    "var s=document.getElementById('inv_status');"
    "s.textContent=T[curLang].imu_inv_saved;"
    "setTimeout(function(){s.textContent='';},3000);}}).catch(function(){});}"
    "function doCalibrate(){"
    "fetch('/calibrate',{method:'POST'}).then(function(r){return r.json();}).then(function(d){"
    "if(d.ok){"
    "document.getElementById('cal_status').textContent=T[curLang].imu_saved;"
    "setTimeout(function(){document.getElementById('cal_status').textContent='';},3000);"
    "reloadIMU();}}).catch(function(){});}"
    "reloadIMU();setInterval(reloadIMU,1000);"
    "</script></div>"
  );

  // ── SICHERHEIT ────────────────────────────────────────────────
  webServer.sendContent(
    "<details class='card'>"
    "<summary><h2 data-i18n='h_sec'>Sicherheit</h2><span class='chev'>&#8250;</span></summary>"
    "<form method='POST' action='/savesec'>"
    "<p style='font-size:.82rem;color:var(--sub);margin-bottom:8px;font-weight:600' data-i18n='sec_ap'>AP-Zugangsdaten</p>"
    "<label data-i18n='l_ap_ssid'>AP Name (SSID)</label>"
    "<input name='as' value='"
  );
  webServer.sendContent(ap_ssid);
  webServer.sendContent(
    "'>"
    "<label data-i18n='l_ap_pass'>AP Passwort (min. 8 Zeichen)</label>"
    "<input type='password' name='ap' placeholder='...' minlength='8'>"
    "<p style='font-size:.82rem;color:var(--sub);margin:18px 0 8px;font-weight:600' data-i18n='sec_portal'>Portal-Zugangsdaten</p>"
    "<label data-i18n='l_pu'>Portal Benutzer</label>"
    "<input name='pu' value='"
  );
  webServer.sendContent(portal_user);
  webServer.sendContent(
    "'>"
    "<label data-i18n='l_pp'>Portal Passwort (min. 6 Zeichen)</label>"
    "<input type='password' name='pp' placeholder='...' minlength='6'>"
    "<label data-i18n='l_pp2'>Passwort best&auml;tigen</label>"
    "<input type='password' name='pp2' placeholder='...' minlength='6'>"
    "<button class='bsec' type='submit' style='margin-top:12px' data-i18n='sec_save'>Speichern &amp; Neustart</button>"
    "</form></details>"
  );

  // ── AKTIONEN ──────────────────────────────────────────────────
  webServer.sendContent(
    String("<div class='card'>"
    "<h2 data-i18n='h_act'>Aktionen</h2>"
    "<form method='POST' action='/testmode'>") +
    (testMode
      ? "<button class='blive' type='submit' data-i18n='to_live'>→ LIVE (echte Sensoren)</button>"
      : "<button class='btest' type='submit' data-i18n='to_test'>→ TEST (Fake-Daten)</button>") +
    "</form>"
    "<form method='POST' action='/reboot'>"
    "<button class='breboot' type='submit' data-i18n='restart'>Neustart</button>"
    "</form></div>"
    "</div>" // .wrap
    "<script>applyLang(curLang);</script>"
    "</body></html>"
  );
}

// ── SAVE KANALCONFIG ─────────────────────────────────────────
void handleSave() {
  if (!requireAuth()) return;
  for (int i = 0; i < 16; i++) {
    kanaele[i].sensor  = webServer.arg("s" + String(i));
    kanaele[i].faktor  = webServer.arg("f" + String(i)).toFloat();
    kanaele[i].offset  = webServer.arg("o" + String(i)).toFloat();
    kanaele[i].einheit = webServer.arg("e" + String(i));
    kanaele[i].aktiv   = webServer.hasArg("c" + String(i));
    kanaele[i].topic   = webServer.arg("t" + String(i));
  }
  saveConfig();
  webServer.sendHeader("Location", "/");
  webServer.send(303);
}

// ── SAVE NETZWERK-CONFIG ──────────────────────────────────────
void handleSaveNet() {
  if (!requireAuth()) return;
  if (webServer.hasArg("ssid") && webServer.arg("ssid").length())
    strncpy(wifi_ssid,   webServer.arg("ssid").c_str(),  sizeof(wifi_ssid)   - 1);
  if (webServer.hasArg("wpass") && webServer.arg("wpass").length())
    strncpy(wifi_pass,   webServer.arg("wpass").c_str(), sizeof(wifi_pass)   - 1);
  if (webServer.hasArg("mqtt") && webServer.arg("mqtt").length())
    strncpy(mqtt_server, webServer.arg("mqtt").c_str(),  sizeof(mqtt_server) - 1);
  if (webServer.hasArg("mport") && webServer.arg("mport").length())
    strncpy(mqtt_port,   webServer.arg("mport").c_str(), sizeof(mqtt_port)   - 1);
  if (webServer.hasArg("muser"))
    strncpy(mqtt_user,   webServer.arg("muser").c_str(), sizeof(mqtt_user)   - 1);
  if (webServer.hasArg("mpass") && webServer.arg("mpass").length())
    strncpy(mqtt_pass,   webServer.arg("mpass").c_str(), sizeof(mqtt_pass)   - 1);
  saveNetConfig();
  webServer.send(200, "text/html",
    "<html><head><meta charset='UTF-8'>"
    "<meta http-equiv='refresh' content='3;url=/'></head>"
    "<body style='background:#0c0c0e;color:#22c55e;font-family:sans-serif;padding:20px'>"
    "<p>Gespeichert. Neustart... / Saved. Restarting...</p></body></html>");
  delay(1000);
  ESP.restart();
}

// ── SAVE SICHERHEITS-CONFIG ───────────────────────────────────
void handleSaveSec() {
  if (!requireAuth()) return;
  String as_ = webServer.arg("as");
  String ap_ = webServer.arg("ap");
  String pu  = webServer.arg("pu");
  String pp  = webServer.arg("pp");
  String pp2 = webServer.arg("pp2");

  if (as_.length() > 0)
    as_.toCharArray(ap_ssid, sizeof(ap_ssid));
  if (ap_.length() >= 8)
    ap_.toCharArray(ap_pass, sizeof(ap_pass));
  if (pu.length() > 0)
    pu.toCharArray(portal_user, sizeof(portal_user));
  if (pp.length() >= 6 && pp == pp2)
    pp.toCharArray(portal_pass, sizeof(portal_pass));

  saveNetConfig();
  webServer.send(200, "text/html",
    "<html><head><meta charset='UTF-8'>"
    "<meta http-equiv='refresh' content='3;url=/'></head>"
    "<body style='background:#0c0c0e;color:#22c55e;font-family:sans-serif;padding:20px'>"
    "<p>Gespeichert. Neustart... / Saved. Restarting...</p></body></html>");
  delay(1000);
  ESP.restart();
}

// ── TEST/LIVE UMSCHALTEN ──────────────────────────────────────
void handleTestMode() {
  if (!requireAuth()) return;
  testMode = !testMode;
  saveNetConfig();
  webServer.send(200, "text/html",
    "<html><head><meta charset='UTF-8'>"
    "<meta http-equiv='refresh' content='2;url=/'></head>"
    "<body style='background:#0c0c0e;color:#fafafa;font-family:sans-serif;padding:20px'>"
    "<p>Modus ge&auml;ndert / Mode changed.</p></body></html>");
  delay(1000);
  ESP.restart();
}

// ── NEUSTART ─────────────────────────────────────────────────
void handleReboot() {
  if (!requireAuth()) return;
  webServer.send(200, "text/html",
    "<html><head><meta charset='UTF-8'></head>"
    "<body style='background:#0c0c0e;color:#fafafa;font-family:sans-serif;padding:20px'>"
    "<p>Neustart / Restarting...</p></body></html>");
  delay(500);
  ESP.restart();
}

// ── LIVE-WERTE API ───────────────────────────────────────────
void handleValues() {
  StaticJsonDocument<1024> doc;
  for (int i = 0; i < 16; i++) {
    if (!kanaele[i].aktiv) continue;
    float voltage;
    if (testMode) {
      voltage = 2.5f + sinf(millis() / 3000.0f + i) * 0.3f;
    } else {
      selectChannel(kanaele[i].klemme - 1);
      delayMicroseconds(500);
      voltage = readADS();
    }
    doc[kanaele[i].sensor] = roundf((voltage * kanaele[i].faktor + kanaele[i].offset) * 100.f) / 100.f;
  }
  doc["pitch"] = roundf((pitch - pitch_offset) * (pitch_invert ? -1.0f : 1.0f) * 10.f) / 10.f;
  doc["roll"]  = roundf((roll  - roll_offset)  * (roll_invert  ? -1.0f : 1.0f) * 10.f) / 10.f;
  String out;
  serializeJson(doc, out);
  webServer.send(200, "application/json", out);
}

// ── RAW ADS DIAGNOSE API ─────────────────────────────────────
void handleRaw() {
  StaticJsonDocument<256> doc;
  JsonObject a = doc.createNestedObject("ads1");
  a["ok"] = adsOK;
  if (adsOK) {
    for (int p = 0; p < 4; p++) {
      int16_t raw = ads.readADC_SingleEnded(p);
      a["a" + String(p)] = roundf(ads.computeVolts(raw) * 1000.f) / 1000.f;
    }
  }
  String out;
  serializeJson(doc, out);
  webServer.send(200, "application/json", out);
}

// ── IMU API ──────────────────────────────────────────────────
void handleIMU() {
  float pc = (pitch - pitch_offset) * (pitch_invert ? -1.0f : 1.0f);
  float rc = (roll  - roll_offset)  * (roll_invert  ? -1.0f : 1.0f);
  StaticJsonDocument<256> doc;
  doc["pitch_raw"]    = roundf(pitch * 100.f) / 100.f;
  doc["roll_raw"]     = roundf(roll  * 100.f) / 100.f;
  doc["pitch_corr"]   = roundf(pc * 100.f) / 100.f;
  doc["roll_corr"]    = roundf(rc * 100.f) / 100.f;
  doc["pitch_offset"] = roundf(pitch_offset * 100.f) / 100.f;
  doc["roll_offset"]  = roundf(roll_offset  * 100.f) / 100.f;
  doc["pitch_invert"] = pitch_invert;
  doc["roll_invert"]  = roll_invert;
  doc["bias_x"]       = roundf(gyroBiasX * 10000.f) / 10000.f;
  doc["bias_y"]       = roundf(gyroBiasY * 10000.f) / 10000.f;
  doc["bias_z"]       = roundf(gyroBiasZ * 10000.f) / 10000.f;
  String out;
  serializeJson(doc, out);
  webServer.send(200, "application/json", out);
}

// ── INVERTIERUNG SETZEN ───────────────────────────────────────
void handleSetInvert() {
  if (!requireAuth()) return;
  pitch_invert = webServer.hasArg("pi") && webServer.arg("pi") == "1";
  roll_invert  = webServer.hasArg("ri") && webServer.arg("ri") == "1";
  Preferences prefs;
  prefs.begin("boatopenio", false);
  prefs.putBool("pitch_inv", pitch_invert);
  prefs.putBool("roll_inv",  roll_invert);
  prefs.end();
  StaticJsonDocument<64> doc;
  doc["ok"]           = true;
  doc["pitch_invert"] = pitch_invert;
  doc["roll_invert"]  = roll_invert;
  String out;
  serializeJson(doc, out);
  webServer.send(200, "application/json", out);
}

// ── MONTAGE-OFFSET SETZEN ────────────────────────────────────
void handleCalibrate() {
  if (!requireAuth()) return;
  pitch_offset = pitch;
  roll_offset  = roll;
  Preferences prefs;
  prefs.begin("boatopenio", false);
  prefs.putFloat("pitch_off", pitch_offset);
  prefs.putFloat("roll_off",  roll_offset);
  prefs.end();
  StaticJsonDocument<128> doc;
  doc["ok"]           = true;
  doc["pitch_offset"] = roundf(pitch_offset * 100.f) / 100.f;
  doc["roll_offset"]  = roundf(roll_offset  * 100.f) / 100.f;
  String out;
  serializeJson(doc, out);
  webServer.send(200, "application/json", out);
}

// ── SETUP ────────────────────────────────────────────────────
void setupWebServer() {
  startCaptivePortal();

  webServer.on("/",           HTTP_GET,  handleRoot);
  webServer.on("/setup",      HTTP_GET,  handleSetup);
  webServer.on("/dosetup",    HTTP_POST, handleDoSetup);
  webServer.on("/save",       HTTP_POST, handleSave);
  webServer.on("/savenet",    HTTP_POST, handleSaveNet);
  webServer.on("/savesec",    HTTP_POST, handleSaveSec);
  webServer.on("/testmode",   HTTP_POST, handleTestMode);
  webServer.on("/reboot",     HTTP_POST, handleReboot);
  webServer.on("/api/values", HTTP_GET,  handleValues);
  webServer.on("/api/raw",    HTTP_GET,  handleRaw);
  webServer.on("/api/imu",    HTTP_GET,  handleIMU);
  webServer.on("/calibrate",  HTTP_POST, handleCalibrate);
  webServer.on("/setinvert",  HTTP_POST, handleSetInvert);

  webServer.onNotFound([]() {
    if (setupRequired()) sendRedirect("http://192.168.4.1/setup");
    else                 sendRedirect("http://192.168.4.1/");
  });

  webServer.begin();
  Serial.println("WebServer + Captive Portal gestartet");
  Serial.println("  http://192.168.4.1  (immer, via AP)");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("  http://" + WiFi.localIP().toString() + "  (via Router)");
}

// ── LOOP ─────────────────────────────────────────────────────
void loopWebServer() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}
