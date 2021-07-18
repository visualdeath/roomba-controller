#include "FSBrowser.h"

#ifdef ESP8266
extern ESP8266WebServer webServer;
#elif defined(ESP32)
extern WebServer webServer(80);
#endif
  
extern Roomba roomba;

String getRebootString() {
    return "<html><head><meta http-equiv=\"refresh\" content=\"4; url=/\"/></head><body><font face='arial'><b><h2>Rebooting... returning in 4 seconds</h2></b></font></body></html>";
}

void handleReboot() {
    webServer.send(200, "text/html", getRebootString());
    delay(500);
    ESP.restart();
}

void handleRoombaApi() {
  String action = webServer.pathArg(1);
  if ( action == "reset" ) {
    roomba.reset();
    webServer.send(200, "text/plain", "OK");
    return;
  }

  if ( action == "wake" ) {
    roomba.wake();
    webServer.send(200, "text/plain", "OK");
    return;
  }

  if ( action == "safemode" ) {
    roomba.safeMode();
    webServer.send(200, "text/plain", "OK");
    return;
  }

  if ( action == "fullmode" ) {
    roomba.fullMode();
    webServer.send(200, "text/plain", "OK");
    return;
  }

  if ( action == "power" ) {
    roomba.power();
    webServer.send(200, "text/plain", "OK");
    return;
  }

  if ( action == "dock" ) {
    roomba.dock();
    webServer.send(200, "text/plain", "OK");
    return;
  }

  if ( action == "demo" ) {
    //roomba.demo();
    webServer.send(200, "text/plain", "OK");
    return;
  }

  if ( action == "spot" ) {
    roomba.spot();
    webServer.send(200, "text/plain", "OK");
    return;
  }

  if ( action == "drive" ) {
    //roomba.drive();
    webServer.send(200, "text/plain", "OK");
    return;
  }

  if ( action == "drivedirect" ) {
    //roomba.drivedirect();
    webServer.send(200, "text/plain", "OK");
    return;
  }

  if ( action == "drivers" ) {
    //roomba.drivers();
    webServer.send(200, "text/plain", "OK");
    return;
  }
}

void handleEspApi() {
  String action = webServer.pathArg(1);
  if ( action == "reset" ) {
    handleReboot();
    return;
  }
}

void handleFSApi() {
  String action = webServer.pathArg(1);
  HTTPMethod method = webServer.method();

  if ( action == "status" ) {
    handleFMStatus();
    return;
  }

  if ( action == "list" ) {
    handleFileList();
    return;
  }

  if ( action == "edit" ) {
    if ( method == HTTP_GET ) {
      handleGetEdit();
      return;
    }

    if ( method == HTTP_PUT ) {
      handleFileCreate();
      return;
    }

    if ( method == HTTP_DELETE ) {
      handleFileDelete();
      return;
    }

    if ( method == HTTP_POST ) {
      replyOK();
      handleFileUpload();
    }
  }
}

void handleApi() {
  String device = webServer.pathArg(0);
  device.toLowerCase();

  if ( device == "roomba" ) {
    handleRoombaApi();
  } else if ( device == "esp" ) {
    handleEspApi();
  } else if ( device == "fs" ) {
    handleFSApi();
  }
}
