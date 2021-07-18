// used when hosting the site on the ESP8266
var address = location.hostname;
var host = location.host;
var urlBase = "";

// used when hosting the site somewhere other than the ESP8266 (handy for testing without waiting forever to upload to SPIFFS)
// var address = "esp8266-1920f7.local";
// var urlBase = "http://" + address + "/";

$(document).ready(function() {
  $("#status").html("Connecting, please wait...");

  $.get(urlBase + "config.json", function(data) {
      $("#status").html("Loading, please wait...");

      $.each(data, function(index, field) {
        if (field.type == "Number") {
          //addNumberField(field);
        } else if (field.type == "Boolean") {
          //addBooleanField(field);
        } else if (field.type == "Select") {
          //addSelectField(field);
        } else if (field.type == "Color") {
          //addColorFieldPalette(field);
          //addColorFieldPicker(field);
        } else if (field.type == "Section") {
          //addSectionField(field);
        } else if (field.type == "Setting") {
          handleSetting(field);
        }
      });

      $("#status").html("Ready");
    })
    .fail(function(jqXHR, textStatus, error) {
      console.log("Request failed: " + textStatus + " responseText: " + jqXHR.responseText);
    });
});

function handleSetting(config) {

  for (let [key, value] of Object.entries(config.value)) {
    if (key.includes("Support") && value == true) {
      $("#" + key + "Entry").removeClass("hidden");
    } else {
      $("#input-" + key).val(value);
    }
  }

  $("#btnOnmqtt").attr("class", config.value.mqttEnabled ? "btn btn-primary" : "btn btn-default");
  $("#btnOffmqtt").attr("class", !config.value.mqttEnabled ? "btn btn-primary" : "btn btn-default");

  $("#btnOnmqtt").click(function() {
    $("#btnOnmqtt").attr("class", "btn btn-primary");
    $("#btnOffmqtt").attr("class", "btn btn-default");
    $("#input-mqttEnabled").val("1");
  });
  $("#btnOffmqtt").click(function() {
    $("#btnOnmqtt").attr("class", "btn btn-default");
    $("#btnOffmqtt").attr("class", "btn btn-primary");
    $("#input-mqttEnabled").val("0");
  });

}