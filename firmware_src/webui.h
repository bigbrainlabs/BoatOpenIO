// ============================================================
// BoatOpenIO – Web-UI & Captive Portal
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

// Ersteinrichtung noch nicht abgeschlossen?
static bool setupRequired() {
  return strlen(portal_pass) == 0;
}

// HTTP Basic Auth prüfen – gibt false zurück und sendet 401 wenn nicht authentifiziert
static bool requireAuth() {
  if (!webServer.authenticate(portal_user, portal_pass)) {
    webServer.requestAuthentication(BASIC_AUTH, "BoatOpenIO");
    return false;
  }
  return true;
}

// AP-Passwort noch auf Default?
static bool isDefaultApPass() {
  return strcmp(ap_pass, AP_DEFAULT_PASS) == 0;
}

// ── ERSTEINRICHTUNG ──────────────────────────────────────────
// Wird beim ersten Boot angezeigt, solange kein portal_pass gesetzt ist.
// Keine Auth erforderlich – es gibt noch kein Passwort.
void handleSetup() {
  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  webServer.send(200, "text/html", "");
  webServer.sendContent(
    "<!DOCTYPE html><html><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>BoatOpenIO – Setup</title>"
    "<style>"
    "body{font-family:sans-serif;max-width:480px;margin:40px auto;padding:0 16px;"
    "color:#eee;background:#1a1a2e}"
    "h1{color:#00d4ff}h2{color:#ffaa00;font-size:1em}"
    ".card{background:#16213e;padding:16px;border-radius:8px;margin:12px 0}"
    "label{display:block;margin-top:8px;font-size:.9em;color:#aaa}"
    "input{background:#0f3460;color:#eee;border:1px solid #444;padding:6px;"
    "width:100%;box-sizing:border-box;border-radius:4px;margin-top:2px}"
    "button{padding:10px;margin-top:12px;width:100%;border:none;border-radius:5px;"
    "background:#006622;color:#fff;font-size:1em;cursor:pointer}"
    ".warn{background:#3a1a00;border:1px solid #ffaa00;padding:10px;"
    "border-radius:6px;color:#ffaa00;margin-bottom:12px;font-size:.9em}"
    ".err{color:#ff4444;font-size:.85em;margin-top:6px}"
    "</style></head><body>"
    "<h1>&#9875; BoatOpenIO</h1>"
    "<div class='warn'>&#9888; "
  );
  webServer.sendContent(
    "First-time setup required. Set a portal password to secure the configuration interface. "
    "<br><br>"
    "Ersteinrichtung erforderlich. Bitte ein Portal-Passwort setzen."
    "</div>"
    "<div class='card'>"
    "<h2>Portal Access / Portal-Zugang</h2>"
    "<form method='POST' action='/dosetup'>"
    "<label>User / Benutzer</label>"
    "<input name='pu' value='admin' required>"
    "<label>Password / Passwort</label>"
    "<input type='password' name='pp' required minlength='6' placeholder='min. 6 characters'>"
    "<label>Confirm Password / Passwort bestätigen</label>"
    "<input type='password' name='pp2' required minlength='6' placeholder='repeat'>"
    "<h2 style='margin-top:16px'>AP Credentials / AP-Zugangsdaten</h2>"
    "<label>AP Name (SSID)</label>"
    "<input name='as' value='" + String(ap_ssid) + "' required>"
    "<label>AP Password / AP-Passwort (min. 8 characters)</label>"
    "<input type='password' name='ap' value='" + String(ap_pass) + "' required minlength='8'>"
    "<button type='submit'>Complete Setup / Einrichtung abschlie&szlig;en</button>"
    "</form></div>"
    "</body></html>"
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
      "<meta http-equiv='refresh' content='2;url=/setup'></head><body>"
      "<p style='color:#f44'>Invalid input. / Ungültige Eingabe.</p></body></html>");
    return;
  }
  pu.toCharArray(portal_user, sizeof(portal_user));
  pp.toCharArray(portal_pass, sizeof(portal_pass));
  as_.toCharArray(ap_ssid,    sizeof(ap_ssid));
  ap_.toCharArray(ap_pass,    sizeof(ap_pass));
  saveNetConfig();
  webServer.send(200, "text/html",
    "<html><head><meta charset='UTF-8'></head><body>"
    "<p>Setup complete. Restarting... / Einrichtung abgeschlossen. Neustart...</p>"
    "</body></html>");
  delay(1000);
  ESP.restart();
}

// ── ROOT ─────────────────────────────────────────────────────
void handleRoot() {
  if (setupRequired()) { sendRedirect("/setup"); return; }
  if (!requireAuth())  return;

  webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  webServer.send(200, "text/html", "");

  // ─ HEAD: CSS + i18n-Script ───────────────────────────────
  webServer.sendContent(
    "<!DOCTYPE html><html><head>"
    "<meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>BoatOpenIO</title>"
    "<style>"
    "body{font-family:sans-serif;max-width:920px;margin:20px auto;padding:0 12px;color:#eee;background:#1a1a2e}"
    "h1{color:#00d4ff;margin:0}h2{color:#00d4ff;font-size:1em;margin:10px 0 5px}"
    ".hdr{display:flex;justify-content:space-between;align-items:center;margin-bottom:8px}"
    ".card{background:#16213e;padding:12px 16px;margin:10px 0;border-radius:8px}"
    "table{width:100%;border-collapse:collapse;font-size:.82em}"
    "th{background:#0f3460;padding:6px;text-align:left}"
    "td{padding:3px 4px;border-bottom:1px solid #2a2a4a}"
    "input,select{background:#0f3460;color:#eee;border:1px solid #444;padding:3px;width:100%;box-sizing:border-box}"
    "input[type=checkbox]{width:auto}"
    "button{padding:9px 12px;margin:4px 0;border:none;border-radius:5px;font-size:.95em;cursor:pointer;width:100%}"
    ".save{background:#006622;color:#fff}.reboot{background:#990000;color:#fff}"
    ".cal{background:#004488;color:#fff}.sec{background:#333300;color:#fff}"
    ".ton{background:#cc6600;color:#fff}.toff{background:#336600;color:#fff}"
    ".ok{color:#00ff88}.err{color:#ff4444}.warn{color:#ffaa00}"
    ".warnbox{background:#3a1a00;border:1px solid #ffaa00;padding:8px 12px;"
    "border-radius:6px;color:#ffaa00;margin:6px 0;font-size:.88em}"
    "#lbtn{width:auto;padding:5px 14px;background:#0f3460;color:#eee;"
    "border:1px solid #555;border-radius:5px;font-size:.9em;cursor:pointer}"
    "#cal_status{font-size:.85em;color:#00ff88;margin-top:4px;min-height:1em}"
    "</style>"
    "<script>"
    "var T={"
      "de:{"
        "h_wifi:'WiFi & MQTT',h_ch:'Kanalzuordnung',h_cal:'Kalibrierungs-Hilfsrechner',"
        "h_live:'Live-Werte',h_diag:'ADS Diagnose (Rohdaten)',"
        "h_imu:'IMU Kalibrierung',h_sec:'Sicherheit',h_act:'Aktionen',"
        "l_ssid:'WiFi SSID',l_wpass:'WiFi Passwort',l_mqtt:'MQTT Server',"
        "l_mport:'MQTT Port',l_muser:'MQTT User',l_mpass:'MQTT Passwort',"
        "save_net:'Speichern & Neustart',"
        "th_sensor:'Sensor',th_ads:'ADS',th_pin:'Pin',th_factor:'Faktor',"
        "th_offset:'Offset',th_unit:'Einheit',th_active:'Aktiv',"
        "save_ch:'Konfiguration speichern',"
        "cal_hint:'Faktor = Echter_Wert / ADS-Pin-Spannung',"
        "l_real:'Echter Wert (Multimeter)',l_adsv:'ADS-Pin-Spannung [V]',l_factor:'Faktor',"
        "th_val:'Wert',loading:'Lade...',not_found:'nicht gefunden',"
        "imu_hint:'Boot auf ruhigem Wasser ausrichten, dann Null setzen.',"
        "imu_raw:'Rohwert (Gyro-Bias korr.)',imu_corr:'Korrigiert',imu_off:'Offset',"
        "imu_bias:'Gyro-Bias (rad/s)',imu_btn:'Jetzt Null setzen',imu_saved:'Offset gespeichert',"
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
        "h_live:'Live Values',h_diag:'ADS Diagnostics (Raw Data)',"
        "h_imu:'IMU Calibration',h_sec:'Security',h_act:'Actions',"
        "l_ssid:'WiFi SSID',l_wpass:'WiFi Password',l_mqtt:'MQTT Server',"
        "l_mport:'MQTT Port',l_muser:'MQTT User',l_mpass:'MQTT Password',"
        "save_net:'Save & Restart',"
        "th_sensor:'Sensor',th_ads:'ADS',th_pin:'Pin',th_factor:'Factor',"
        "th_offset:'Offset',th_unit:'Unit',th_active:'Active',"
        "save_ch:'Save Configuration',"
        "cal_hint:'Factor = Real_Value / ADS_Pin_Voltage',"
        "l_real:'Real Value (Multimeter)',l_adsv:'ADS Pin Voltage [V]',l_factor:'Factor',"
        "th_val:'Value',loading:'Loading...',not_found:'not found',"
        "imu_hint:'Level the boat on calm water, then set zero.',"
        "imu_raw:'Raw (gyro bias corrected)',imu_corr:'Corrected',imu_off:'Offset',"
        "imu_bias:'Gyro Bias (rad/s)',imu_btn:'Set Zero Now',imu_saved:'Offset saved',"
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
      "localStorage.setItem('lang',l);"
      "curLang=l;"
      "var b=document.getElementById('lbtn');"
      "if(b)b.textContent=l==='de'?'EN':'DE';"
    "}"
    "function toggleLang(){applyLang(curLang==='de'?'en':'de');}"
    "</script>"
    "</head><body>"
    "<div class='hdr'>"
    "<h1>&#9875; BoatOpenIO</h1>"
    "<button id='lbtn' onclick='toggleLang()'>EN</button>"
    "</div>"
  );

  // ─ Warnung: Default-AP-Passwort noch aktiv ───────────────
  if (isDefaultApPass()) {
    webServer.sendContent(
      "<div class='warnbox'>&#9888; <span data-i18n='warn_defpass'>"
      "Standard-Passwort aktiv – bitte AP-Passwort ändern!</span></div>"
    );
  }

  // ─ Status-Card ───────────────────────────────────────────
  bool wifiUp = (WiFi.status() == WL_CONNECTED);
  webServer.sendContent(
    String("<div class='card'>") +
    "<b>AP:</b> " + ap_ssid + " &rarr; 192.168.4.1<br>" +
    "<b>WiFi:</b> " + (wifiUp
      ? WiFi.SSID() + " (" + WiFi.RSSI() + " dBm) | " + WiFi.localIP().toString()
      : "<span class='warn' data-i18n='s_nowifi'>nicht verbunden</span>") + "<br>" +
    "<b>MQTT:</b> " + mqtt_server + ":" + mqtt_port + " &ndash; " +
    (mqtt.connected()
      ? "<span class='ok'  data-i18n='s_on' >verbunden</span>"
      : "<span class='err' data-i18n='s_off'>getrennt</span>") + "<br>" +
    "<b data-i18n='s_mode'>Modus</b>: " +
    (testMode ? "<span style='color:#fa0'>TEST</span>" : "<span class='ok'>LIVE</span>") +
    " | <b>MPU6050:</b> " +
    (mpuOK ? "<span class='ok'>OK</span>"
           : "<span class='err' data-i18n='s_miss'>fehlt</span>") + "<br>" +
    "<b>ADS:</b> " +
    (adsOK[0] ? "<span class='ok'>1 </span>" : "<span class='err'>1 </span>") +
    (adsOK[1] ? "<span class='ok'>2 </span>" : "<span class='err'>2 </span>") +
    (adsOK[2] ? "<span class='ok'>3 </span>" : "<span class='err'>3 </span>") +
    (adsOK[3] ? "<span class='ok'>4</span>"  : "<span class='err'>4</span>") + "<br>" +
    "<b data-i18n='s_uptime'>Uptime</b>: " + String(millis() / 1000) +
    " s | Heap: " + ESP.getFreeHeap() + " B"
    "</div>"
  );

  // ─ WiFi & MQTT Card ──────────────────────────────────────
  webServer.sendContent(
    String("<div class='card'>"
    "<h2 data-i18n='h_wifi'>WiFi &amp; MQTT</h2>"
    "<form method='POST' action='/savenet'>") +
    "<label data-i18n='l_ssid'>WiFi SSID</label>"
    "<input name='ssid' value='" + wifi_ssid + "' autocomplete='off'>" +
    "<label data-i18n='l_wpass'>WiFi Passwort</label>"
    "<input type='password' name='wpass' placeholder='...'>" +
    "<label data-i18n='l_mqtt'>MQTT Server</label>"
    "<input name='mqtt' value='" + mqtt_server + "'>" +
    "<label data-i18n='l_mport'>MQTT Port</label>"
    "<input name='mport' value='" + mqtt_port + "'>" +
    "<label data-i18n='l_muser'>MQTT User</label>"
    "<input name='muser' value='" + mqtt_user + "'>" +
    "<label data-i18n='l_mpass'>MQTT Passwort</label>"
    "<input type='password' name='mpass' placeholder='...'>" +
    "<button class='save' type='submit' data-i18n='save_net'>Speichern &amp; Neustart</button>"
    "</form></div>"
  );

  // ─ Kanal-Tabelle ─────────────────────────────────────────
  webServer.sendContent(
    "<div class='card'>"
    "<h2 data-i18n='h_ch'>Kanalzuordnung</h2>"
    "<form method='POST' action='/save'>"
    "<table><tr>"
    "<th>#</th>"
    "<th data-i18n='th_sensor'>Sensor</th>"
    "<th data-i18n='th_ads'>ADS</th>"
    "<th data-i18n='th_pin'>Pin</th>"
    "<th data-i18n='th_factor'>Faktor</th>"
    "<th data-i18n='th_offset'>Offset</th>"
    "<th data-i18n='th_unit'>Einheit</th>"
    "<th data-i18n='th_active'>Aktiv</th>"
    "</tr>"
  );
  for (int i = 0; i < 16; i++) {
    String r = "<tr><td>" + String(i + 1) + "</td>";
    r += "<td><input name='s" + String(i) + "' value='" + kanaele[i].sensor + "'></td>";
    r += "<td><select name='a" + String(i) + "'>";
    for (int a = 1; a <= 4; a++)
      r += "<option value='" + String(a) + "'" + (kanaele[i].ads == a ? " selected" : "") + ">" + a + "</option>";
    r += "</select></td><td><select name='p" + String(i) + "'>";
    for (int p = 0; p <= 3; p++)
      r += "<option value='" + String(p) + "'" + (kanaele[i].pin == p ? " selected" : "") + ">A" + p + "</option>";
    r += "</select></td>";
    r += "<td><input name='f" + String(i) + "' value='" + String(kanaele[i].faktor, 4) + "' style='width:65px'></td>";
    r += "<td><input name='o" + String(i) + "' value='" + String(kanaele[i].offset, 4) + "' style='width:65px'></td>";
    r += "<td><input name='e" + String(i) + "' value='" + kanaele[i].einheit + "' style='width:45px'></td>";
    r += "<td><input type='checkbox' name='c" + String(i) + "'" + (kanaele[i].aktiv ? " checked" : "") + "></td></tr>";
    webServer.sendContent(r);
  }
  webServer.sendContent(
    "</table>"
    "<button class='save' type='submit' data-i18n='save_ch'>Konfiguration speichern</button>"
    "</form></div>"
  );

  // ─ Kalibrierungs-Hilfsrechner ────────────────────────────
  webServer.sendContent(
    "<div class='card'>"
    "<h2 data-i18n='h_cal'>Kalibrierungs-Hilfsrechner</h2>"
    "<p style='font-size:.85em;color:#aaa' data-i18n='cal_hint'>Faktor = Echter_Wert / ADS-Pin-Spannung</p>"
    "<label data-i18n='l_real'>Echter Wert (Multimeter)</label>"
    "<input type='number' step='0.001' id='cr' oninput='cf()'>"
    "<label data-i18n='l_adsv'>ADS-Pin-Spannung [V]</label>"
    "<input type='number' step='0.001' id='cp' oninput='cf()'>"
    "<label data-i18n='l_factor'>Faktor</label>"
    "<input id='cres' readonly>"
    "<script>"
    "function cf(){"
      "var r=parseFloat(document.getElementById('cr').value),"
      "p=parseFloat(document.getElementById('cp').value);"
      "if(!isNaN(r)&&!isNaN(p)&&p!==0)"
      "document.getElementById('cres').value=(r/p).toFixed(4);"
    "}"
    "</script></div>"
  );

  // ─ Live-Werte ─────────────────────────────────────────────
  webServer.sendContent(
    "<div class='card'>"
    "<h2 data-i18n='h_live'>Live-Werte</h2>"
    "<div id='vals'><small data-i18n='loading'>Lade...</small></div>"
    "<script>"
    "function reloadVals(){"
      "fetch('/api/values').then(function(r){return r.json();}).then(function(d){"
        "var t=T[curLang];"
        "var s='<table><tr><th>Sensor</th><th>'+t.th_val+'</th></tr>';"
        "for(var k in d)s+='<tr><td>'+k+'</td><td>'+parseFloat(d[k]).toFixed(2)+'</td></tr>';"
        "s+='</table>';"
        "document.getElementById('vals').innerHTML=s;"
      "}).catch(function(){});"
    "}"
    "reloadVals();setInterval(reloadVals,3000);"
    "</script></div>"
  );

  // ─ ADS Diagnose ──────────────────────────────────────────
  webServer.sendContent(
    "<div class='card'>"
    "<h2 data-i18n='h_diag'>ADS Diagnose (Rohdaten)</h2>"
    "<div id='raw'><small data-i18n='loading'>Lade...</small></div>"
    "<script>"
    "function reloadRaw(){"
      "fetch('/api/raw').then(function(r){return r.json();}).then(function(d){"
        "var t=T[curLang];"
        "var s='<table><tr><th>ADS</th><th>Addr</th><th>A0</th><th>A1</th><th>A2</th><th>A3</th></tr>';"
        "for(var i=1;i<=4;i++){"
          "var k='ads'+i,a=d[k];"
          "if(!a)continue;"
          "s+='<tr>';"
          "s+='<td>'+(a.ok?'<span class=ok>ADS'+i+'</span>':'<span class=err>ADS'+i+'</span>')+'</td>';"
          "s+='<td>'+a.addr+'</td>';"
          "if(a.ok){"
            "s+='<td>'+a.a0.toFixed(3)+'V</td>';"
            "s+='<td>'+a.a1.toFixed(3)+'V</td>';"
            "s+='<td>'+a.a2.toFixed(3)+'V</td>';"
            "s+='<td>'+a.a3.toFixed(3)+'V</td>';"
          "}else{"
            "s+='<td colspan=4 style=\"color:#f44\">'+t.not_found+'</td>';"
          "}"
          "s+='</tr>';"
        "}"
        "s+='</table>';"
        "document.getElementById('raw').innerHTML=s;"
      "}).catch(function(){});"
    "}"
    "reloadRaw();setInterval(reloadRaw,2000);"
    "</script></div>"
  );

  // ─ IMU Kalibrierung ──────────────────────────────────────
  webServer.sendContent(
    "<div class='card'>"
    "<h2 data-i18n='h_imu'>IMU Kalibrierung</h2>"
    "<p style='font-size:.85em;color:#aaa' data-i18n='imu_hint'>"
    "Boot auf ruhigem Wasser ausrichten, dann Null setzen.</p>"
    "<div id='imu_table'><small data-i18n='loading'>Lade...</small></div>"
    "<button class='cal' onclick='doCalibrate()' data-i18n='imu_btn'>Jetzt Null setzen</button>"
    "<div id='cal_status'></div>"
    "<script>"
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
              "<td style=\"color:#aaa\">'+d.pitch_offset.toFixed(2)+'&deg;</td>"
              "<td style=\"color:#aaa\">'+d.roll_offset.toFixed(2)+'&deg;</td></tr>';"
        "s+='</table>';"
        "s+='<small style=\"color:#555;margin-top:4px;display:block\">"
             "'+t.imu_bias+': X='+d.bias_x.toFixed(4)+' Y='+d.bias_y.toFixed(4)+' Z='+d.bias_z.toFixed(4)+'</small>';"
        "document.getElementById('imu_table').innerHTML=s;"
      "}).catch(function(){});"
    "}"
    "function doCalibrate(){"
      "fetch('/calibrate',{method:'POST'}).then(function(r){return r.json();}).then(function(d){"
        "if(d.ok){"
          "document.getElementById('cal_status').textContent=T[curLang].imu_saved;"
          "setTimeout(function(){document.getElementById('cal_status').textContent='';},3000);"
          "reloadIMU();"
        "}"
      "}).catch(function(){});"
    "}"
    "reloadIMU();setInterval(reloadIMU,1000);"
    "</script></div>"
  );

  // ─ Sicherheit ────────────────────────────────────────────
  webServer.sendContent(
    String("<div class='card'>"
    "<h2 data-i18n='h_sec'>Sicherheit</h2>"
    "<form method='POST' action='/savesec'>"
    "<b data-i18n='sec_ap'>AP-Zugangsdaten</b><br><br>"
    "<label data-i18n='l_ap_ssid'>AP Name (SSID)</label>"
    "<input name='as' value='" + ap_ssid + "'>" +
    "<label data-i18n='l_ap_pass'>AP Passwort (min. 8 Zeichen)</label>"
    "<input type='password' name='ap' placeholder='...' minlength='8'>"
    "<br><b data-i18n='sec_portal'>Portal-Zugangsdaten</b><br><br>"
    "<label data-i18n='l_pu'>Portal Benutzer</label>"
    "<input name='pu' value='" + portal_user + "'>" +
    "<label data-i18n='l_pp'>Portal Passwort (min. 6 Zeichen)</label>"
    "<input type='password' name='pp' placeholder='...' minlength='6'>"
    "<label data-i18n='l_pp2'>Passwort bestätigen</label>"
    "<input type='password' name='pp2' placeholder='...' minlength='6'>"
    "<button class='sec' type='submit' data-i18n='sec_save'>Speichern &amp; Neustart</button>"
    "</form></div>"
  );

  // ─ Aktionen ──────────────────────────────────────────────
  webServer.sendContent(
    String("<div class='card'>"
    "<h2 data-i18n='h_act'>Aktionen</h2>"
    "<form method='POST' action='/testmode'>") +
    (testMode
      ? "<button class='toff' type='submit' data-i18n='to_live'>→ LIVE (echte Sensoren)</button>"
      : "<button class='ton'  type='submit' data-i18n='to_test'>→ TEST (Fake-Daten)</button>") +
    "</form>"
    "<form method='POST' action='/reboot'>"
    "<button class='reboot' type='submit' data-i18n='restart'>Neustart</button>"
    "</form></div>"
    "<script>applyLang(curLang);</script>"
    "</body></html>"
  );
}

// ── SAVE KANALCONFIG ─────────────────────────────────────────
void handleSave() {
  if (!requireAuth()) return;
  for (int i = 0; i < 16; i++) {
    kanaele[i].sensor  = webServer.arg("s" + String(i));
    kanaele[i].ads     = webServer.arg("a" + String(i)).toInt();
    kanaele[i].pin     = webServer.arg("p" + String(i)).toInt();
    kanaele[i].faktor  = webServer.arg("f" + String(i)).toFloat();
    kanaele[i].offset  = webServer.arg("o" + String(i)).toFloat();
    kanaele[i].einheit = webServer.arg("e" + String(i));
    kanaele[i].aktiv   = webServer.hasArg("c" + String(i));
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
    "<meta http-equiv='refresh' content='3;url=/'></head><body>"
    "<p>Gespeichert / Saved. Neustart / Restarting...</p></body></html>");
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
    "<meta http-equiv='refresh' content='3;url=/'></head><body>"
    "<p>Gespeichert / Saved. Neustart / Restarting...</p></body></html>");
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
    "<meta http-equiv='refresh' content='2;url=/'></head><body>"
    "<p>Modus ge&auml;ndert / Mode changed.</p></body></html>");
  delay(1000);
  ESP.restart();
}

// ── NEUSTART ─────────────────────────────────────────────────
void handleReboot() {
  if (!requireAuth()) return;
  webServer.send(200, "text/html",
    "<html><head><meta charset='UTF-8'></head><body>"
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
      voltage = readADS(kanaele[i].ads, kanaele[i].pin);
    }
    doc[kanaele[i].sensor] = roundf((voltage * kanaele[i].faktor + kanaele[i].offset) * 100.f) / 100.f;
  }
  doc["pitch"] = roundf((pitch - pitch_offset) * 10.f) / 10.f;
  doc["roll"]  = roundf((roll  - roll_offset)  * 10.f) / 10.f;
  String out;
  serializeJson(doc, out);
  webServer.send(200, "application/json", out);
}

// ── RAW ADS DIAGNOSE API ─────────────────────────────────────
void handleRaw() {
  const char* addrStr[4] = {"0x48", "0x49", "0x4A", "0x4B"};
  StaticJsonDocument<512> doc;
  for (int i = 0; i < 4; i++) {
    JsonObject a = doc.createNestedObject(String("ads") + (i + 1));
    a["ok"]   = adsOK[i];
    a["addr"] = addrStr[i];
    if (adsOK[i]) {
      for (int p = 0; p < 4; p++) {
        int16_t raw = ads[i].readADC_SingleEnded(p);
        a["a" + String(p)] = roundf(ads[i].computeVolts(raw) * 1000.f) / 1000.f;
      }
    }
  }
  String out;
  serializeJson(doc, out);
  webServer.send(200, "application/json", out);
}

// ── IMU KALIBRIERUNG API ─────────────────────────────────────
void handleIMU() {
  StaticJsonDocument<256> doc;
  doc["pitch_raw"]    = roundf(pitch * 100.f) / 100.f;
  doc["roll_raw"]     = roundf(roll  * 100.f) / 100.f;
  doc["pitch_corr"]   = roundf((pitch - pitch_offset) * 100.f) / 100.f;
  doc["roll_corr"]    = roundf((roll  - roll_offset)  * 100.f) / 100.f;
  doc["pitch_offset"] = roundf(pitch_offset * 100.f) / 100.f;
  doc["roll_offset"]  = roundf(roll_offset  * 100.f) / 100.f;
  doc["bias_x"]       = roundf(gyroBiasX * 10000.f) / 10000.f;
  doc["bias_y"]       = roundf(gyroBiasY * 10000.f) / 10000.f;
  doc["bias_z"]       = roundf(gyroBiasZ * 10000.f) / 10000.f;
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
