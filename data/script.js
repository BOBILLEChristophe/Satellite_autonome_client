// https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API/Writing_WebSocket_client_applications



const gateway = 'ws://'+window.location.hostname+'/ws';
var websocket;
function initWebSocket() {
  console.log('Trying to open a WebSocket connection...');
  websocket = new WebSocket(gateway);
  websocket.onopen    = onOpen;
  websocket.onclose   = onClose;
  websocket.onmessage = onMessage; // <-- add this line
}
//websocket.onopen = (event) => {
function onOpen(event) {
  console.log('Connection opened');
  document.getElementById('messages').innerHTML = "Connected";
}
//websocket.onClose = (event) => {
function onClose(event) {
  console.log('Connection closed');
  document.getElementById('messages').innerHTML = "Connection closed";
  setTimeout(initWebSocket, 2000);
}
//websocket.onMessage = (event) => {
function onMessage(event) {
  console.log(`Received a notification from ${event.origin}`);
  console.log('Server: ', event.data);
  //document.getElementById('messages').innerHTML = event.data;

  //-> en JSON
  let data = JSON.parse(event.data);
  document.getElementById('idNode').value = data.idNode;
  document.getElementById('p00').value = data.p00;
  document.getElementById('p01').value = data.p01;
  document.getElementById('p10').value = data.p10;
  document.getElementById('p11').value = data.p11;
  document.getElementById('m00').value = data.m00;
  document.getElementById('m01').value = data.m01;
  document.getElementById('m10').value = data.m10;
  document.getElementById('m11').value = data.m11;

  // Aiguilles
  document.getElementById('s0').value = data.s0;
  document.getElementById('ss00').value = data.ss00;
  document.getElementById('ss01').value = data.ss01;
  document.getElementById('ss02').value = data.ss02;
  document.getElementById('s1').value = data.s1;
  document.getElementById('ss10').value = data.ss10;
  document.getElementById('ss11').value = data.ss11;
  document.getElementById('ss12').value = data.ss12;
  document.getElementById('s2').value = data.s2;
  document.getElementById('ss20').value = data.ss20;
  document.getElementById('ss21').value = data.ss21;
  document.getElementById('ss22').value = data.ss22;
  document.getElementById('s3').value = data.s3;
  document.getElementById('ss30').value = data.ss30;
  document.getElementById('ss31').value = data.ss31;
  document.getElementById('ss32').value = data.ss32;
  document.getElementById('s4').value = data.s4;
  document.getElementById('ss40').value = data.ss40;
  document.getElementById('ss41').value = data.ss41;
  document.getElementById('ss42').value = data.ss42;
  document.getElementById('s5').value = data.s5;
  document.getElementById('ss50').value = data.ss50;
  document.getElementById('ss51').value = data.ss51;
  document.getElementById('ss52').value = data.ss52;

  document.getElementById('discovery_on').checked = data.discovery_on;
  document.getElementById('wifi_on').checked = data.wifi_on;

  document.getElementById('rawData').innerHTML = event.data;
  // Voir côté serveur ESP32
  /*
    const uint8_t size = JSON_OBJECT_SIZE(1);
    StaticJsonDocument<size> doc;
    doc["idNode"] = node->gId();
    char data[17];
    size_t len = serializeJson(doc, data);
    _ws->textAll(data, len);
  */
}

window.addEventListener('load', onLoad);

function onLoad(event) {
  initWebSocket();
  //initJson();
}


//var initJson = function() {
//    fetch("param.jso")
//    .then(response => {
//     return response.json();
//     })
//    .then(jsondata => parseEventJson(jsondata),initDivProg());
//  }


//fonction OK
function sendJson() {
// Construct a msg object containing the data the server needs to process the message from the chat client.
const msg = {
  idNode: document.getElementById('idNode').value,
  comptAig: document.getElementById('comptAig').value,
  p00: document.getElementById('p00').value,
  p01: document.getElementById('p01').value,
  p10: document.getElementById('p10').value,
  p11: document.getElementById('p11').value,
  m00: document.getElementById('m00').value,
  m01: document.getElementById('m01').value,
  m10: document.getElementById('m10').value,
  m11: document.getElementById('m11').value

  //date: Date.now()
};
// Send the msg object as a JSON-formatted string.
websocket.send(JSON.stringify(msg));
}

function servoSettings(obj)
{
const msg = {
  "servoSettings":[obj.id, obj.value, obj.name]
}
websocket.send(JSON.stringify(msg));
}

function servoTest(obj)
{
const msg = {
  "servoTest":[obj.name]
}
websocket.send(JSON.stringify(msg));
}

function wifi_on(obj)
{
const msg = {
  "wifi_on":[obj.checked]
}
websocket.send(JSON.stringify(msg));
}

function discovery_on(obj)
{
const msg = {
  "discovery_on":[obj.checked]
}
websocket.send(JSON.stringify(msg));
}

function restartEsp(obj)
{
const msg = {"restartEsp":[obj.id]}
websocket.send(JSON.stringify(msg));
}

function save(obj)
{
const msg = {"save":[obj.id]}
websocket.send(JSON.stringify(msg));
}


var parseEventJson = function(jsondata) {
console.log(jsondata.varEtatMachine);
let modeExpert = (jsondata.varEtatMachine & 0x40 ) >> 6
let modePtMt = (jsondata.varEtatMachine & 0x20 ) >> 5

let tempDateVal = jsondata.dateValidite;
let indexOf = tempDateVal.indexOf('/');
let jour = tempDateVal.substr(0, indexOf);
tempDateVal = tempDateVal.substr(indexOf+1);
lindexOf = tempDateVal.indexOf('/');
let mois = tempDateVal.substr(0, indexOf);
tempDateVal = tempDateVal.substr(indexOf+1);
let annee = tempDateVal.substr(0);

let dateValidite = annee+'-'+mois+'-'+jour;

document.getElementById('numSerie').value = jsondata.numSerie;
document.getElementById('client').value = jsondata.client;
document.getElementById('dateValidite').value = dateValidite;
document.getElementById('numVersion').value = jsondata.numVersion;
document.getElementById('constHall').value = jsondata.constHall;
document.getElementById('constLux').value = jsondata.constLux;
document.getElementById('constTemp').value = jsondata.constTemp;
document.getElementById('constUv').value = jsondata.constUv;
document.getElementById('constTemp').value = jsondata.constTemp;
document.getElementById('constUv').value = jsondata.constUv;

if(1 == modeExpert)
  document.getElementById('exp_1').checked = 'checked';
else
  document.getElementById('exp_0').checked = 'checked';

if(1 == modePtMt)
  document.getElementById('ptmt_1').checked = 'checked';
else
  document.getElementById('ptmt_0').checked = 'checked';
}


var initDivProg = function() {
console.log('initDivProg');
  /*document.getElementById("a0").addEventListener("click", function() {
  document.getElementById("pgm0").hidden = false;
  document.getElementById("pgm1").hidden = true;
  document.getElementById("pgm2").hidden = true;
  document.getElementById("pgm3").hidden = true;
}, false);

  document.getElementById("a1").addEventListener("click", function() {
  document.getElementById("pgm0").hidden = true;
  document.getElementById("pgm1").hidden = false;
  document.getElementById("pgm2").hidden = true;
  document.getElementById("pgm3").hidden = true;
}, false);

  document.getElementById("a2").addEventListener("click", function() {
  document.getElementById("pgm0").hidden = true;
  document.getElementById("pgm1").hidden = true;
  document.getElementById("pgm2").hidden = false;
  document.getElementById("pgm3").hidden = true;
}, false);

  document.getElementById("a3").addEventListener("click", function() {
  document.getElementById("pgm0").hidden = true;
  document.getElementById("pgm1").hidden = true;
  document.getElementById("pgm2").hidden = true;
  document.getElementById("pgm3").hidden = false;
}, false);*/
}


var submit_0 = function () {
var msgStr = "m ";
var radios = document.getElementsByName('selectMang_0');
var numMangeoire = 0;
var check = 0;
for (var i = 0; i < 4; i++) {
  if (radios[i].checked) {
    numMangeoire = radios[i].value;
    check = 1;
  }
}
if (check == 0) {
  alert("Choissisez une ");
  return;
}
var quantite = document.getElementById('quantite').selectedIndex;
if (quantite == 0) {
  alert("Choissisez une ");
  return;
}
msgStr += Number(numMangeoire);
msgStr += " ";
if (quantite == 10)
  msgStr += Number(quantite);
else
  msgStr += "0" + quantite;
websocket.send(msgStr);
}


var submit_1 = function () {
var msgStr = "";
for (var i = 0; i < 4; i++) {
    for (var j = 0; j < 3; j++) {
       let quantite_ = 'quantite_' + i;
        quantite_ = quantite_ + j;
      var selectElem = document.getElementById(quantite_);
      var quantite = selectElem.selectedIndex;
      //if (quantite > 0) {
        let appt_ = 'appt_' + i;
        appt_ = appt_ + j;
        var horaire = document.getElementById(appt_).value;
        msgStr = "a ";
        msgStr += Number(i);         // Num mangeoire
        msgStr += " ";
        msgStr += Number(j);         // Num du menu
        msgStr += " ";
        if (quantite == 10)
          msgStr += Number(quantite);
        else
          msgStr += "0" + quantite;
        msgStr += " ";
        if(horaire == "--:--")
          horaire = "00:00";
        msgStr += horaire;
        websocket.send(msgStr);
        console.log(msgStr);
      //}
    }
  //}
}
}


