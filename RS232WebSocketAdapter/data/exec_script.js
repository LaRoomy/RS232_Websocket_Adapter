//alert("This alert box was called with the onload event");


var gateway = `ws://${window.location.hostname}/ws`;
var webSocket;
var dataPackageArray = [];
var receiveButtonInStopMode = false;

function initWebSocket(){
    console.log("init WebSocket Session");
    webSocket = new WebSocket(gateway);
    webSocket.onopen = onWebSocketOpen;
    webSocket.onclose = onWebSocketClose;
    webSocket.onmessage = onWebSocketMessage;
}

function onWebSocketOpen(event){
    console.log("WebSocket opened");
}

function onWebSocketClose(event){
    console.log("WebSocket closed - try to re-init in 2000 milliseconds");
    setTimeout(initWebSocket, 2000);
}

function onWebSocketMessage(event){

    if((event.data[0] == '_')&&(event.data[1] == 'C')){
        // this is a config/command transmission

        if((event.data[2] == 'n')&&(event.data[3] == 'x')&&(event.data[4] == 't')){
            // next data package requested
            // get requested index
            var num = "";

            for(var i = 5; i < event.data.length; i++){
                if(event.data[i] == 'E'){
                    break;
                }
                else {
                    num += event.data[i];
                }
            }
            // convert to int
            var rqIndex = parseInt(num);
            // send requested package
            webSocket.send(dataPackageArray[rqIndex]);
        }
        else if((event.data[2] == 'c')&&(event.data[3] == 'o')&&(event.data[4] == 'n')){
            // status message - connected
            notifyUser("Status: Remote socket ready.");
            // request configuration (if this is the config page)
            if(pageType == 'config'){
                webSocket.send("_C000GE");
            }
        }
        else if((event.data[2] == 'u')&&(event.data[3] == 'm')&&(event.data[4] == 'g')){
            // display message to user
            var msg = ""
            for(var i = 5; i < event.data.length; i++){
                msg += event.data[i];
            }
            notifyUser(msg);
        }
        else if((event.data[2] == 'c')&&(event.data[3] == 'r')&&(event.data[4] == 'q')){
            // configuration request response
            readConfigString(event.data);


            // temp:
            //notifyUser(event.data);

        }
        else if((event.data[2] == 's')&&(event.data[3] == 'r')&&(event.data[4] == 's')){
            // reception started message
            document.getElementById('receiveButton').innerHTML = "Stop";
            receiveButtonInStopMode = true;
            notifyUser("Receive Operation started...");
        }
        else if((event.data[2] == 's')&&(event.data[3] == 'r')&&(event.data[4] == 't')){
            // reception terminated
            document.getElementById('receiveButton').innerHTML = "Receive";
            receiveButtonInStopMode = false;
        }
        else if((event.data[2] == 'r')&&(event.data[3] == 'r')&&(event.data[4] == 'r')){
            // data received

            var outString = "";

            for(var i = 5; i < event.data.length; i++){
                outString += event.data[i];
            }
            
            // temp?
            document.getElementById('nc_content_text_area').value = outString;
        }
    }
}

function notifyUser(message){
    var tArea = document.getElementById("log_text_area");
    var pageType = "";
    document.getElementById("log_text_area").value += ('\n' + message);
    tArea.scrollTop = tArea.scrollHeight;
}

function initControls(){
    document.getElementById('sendButton').addEventListener('click', onSendButtonClicked);
    document.getElementById('receiveButton').addEventListener('click', onReceiveButtonClicked);
    document.getElementById('configButton').addEventListener('click', onConfigButtonClicked);
}

function onLoad(event){

    if(document.getElementById('configBody') != null){
        pageType = "config";
    }
    else {
        pageType = "transmission";
        // load content! if there is one!
        var ncContent = localStorage.getItem('ncContent');
        if(ncContent.length > 0){
            document.getElementById('nc_content_text_area').value = localStorage.getItem('ncContent');
        }
    }
    initWebSocket();
    initControls();
}

function onBeforeUnload(){
    if(pageType == 'transmission'){
        localStorage.setItem('ncContent', document.getElementById('nc_content_text_area').value);
    }
}

window.addEventListener('load', onLoad);
window.addEventListener('unload', onBeforeUnload);

function onSendButtonClicked(){

    var tArea = document.getElementById("nc_content_text_area");
    var data = tArea.value;
    var length = tArea.value.length;

    notifyUser("Sending data. Size: " + length);

    sendData('data', data, length);
}

function onReceiveButtonClicked(){
    if(receiveButtonInStopMode){
        webSocket.send("_C000ME");
    }
    else {
        document.getElementById("nc_content_text_area").value = "";
        webSocket.send("_C000XE");
    }
}

function onConfigButtonClicked(){
    location.href = "webSocket_nc_Configuration.html";
}

function onConfigPageBackButtonClicked(){

    // in release-mode
    location.href = "/";

    // for testing purposes!
    //location.href = "webSocket_nc_Transmission.html";
}

function onConfigPageResetButtonClicked(){
    // send reset command
    webSocket.send("_C000RE");
}

function sendData(type, data, length){

    // 5 bytes fixed header length (default header for one segment)
    var header_type_conf = "_C000";
    var header_type_data = "_D000";

    dataPackageArray = [];

    if(length > 500){
        // if the size of data is larger than 500bytes, it must be segmented
        var index = 0;
        var lengthCounter = 0;
        var segmentIndex = 0;
        var dataToSend = "";

        while(index < length){
            
            // max package size reached -> send it
            if(lengthCounter >= 500){
                // first build a simple header for the transmission
                var segmentHeaderString = "";

                // what if this is the last package???

                if(segmentIndex == 0){
                    // this is the first package, apply the appropriate header entry
                    if(type == 'config'){
                        segmentHeaderString = "sC";
                    }
                    else if(type == 'data'){
                        segmentHeaderString = "sD";
                    }
                    else {
                        segmentHeaderString = "sU";
                    }
                }
                else {
                    if(index == length){
                        // must be the last package, apply the appropriate header entry
                        if(type == 'config'){
                            segmentHeaderString = "eC";
                        }
                        else if(type == 'data'){
                            segmentHeaderString = "eD";
                        }
                        else {
                            segmentHeaderString = "eU";
                        }
                    }
                    else {
                        if(type == 'config'){
                            segmentHeaderString = "xC";
                        }
                        else if(type == 'data'){
                            segmentHeaderString = "xD";
                        }
                        else {
                            segmentHeaderString = "xU";
                        }
                    }
                }

                if(segmentIndex < 10){
                    segmentHeaderString += ("00" + segmentIndex);
                }
                else if(segmentIndex < 100){
                    segmentHeaderString += ("0" + segmentIndex);
                }
                else {
                    if(segmentIndex > 999){
                        // error
                        notifyUser("Error: Datasize to large (999). Aborting transmission.");
                    }
                    else {
                        segmentHeaderString += segmentIndex;
                    }
                }                
                //webSocket.send(segmentHeaderString + dataToSend);
                dataPackageArray[segmentIndex] = (segmentHeaderString + dataToSend);
                                
                dataToSend = "";
                lengthCounter = 0;
                segmentIndex++;
            }
            else {
                dataToSend += data[index];

                index++;
                lengthCounter++;
            }
        }

        if(lengthCounter < 500 && lengthCounter > 0){
            // this must be the last package, apply the appropriate header entry
            var sheaderStr = "";

            if(type == 'config'){
                sheaderStr = "eC";
            }
            else if(type == 'data'){
                sheaderStr = "eD";
            }
            else {
                sheaderStr = "eU";
            }

            if(segmentIndex < 10){
                sheaderStr += ("00" + segmentIndex);
            }
            else if(segmentIndex < 100){
                sheaderStr += ("0" + segmentIndex);
            }
            else {
                if(segmentIndex > 999){
                    // error
                    notifyUser("Error: Datasize to large (999). Aborting transmission.");
                }
                else {
                    sheaderStr += segmentIndex;
                }
            }
            //webSocket.send(sheaderStr + dataToSend);
            dataPackageArray[segmentIndex] = (sheaderStr + dataToSend);

            // send first package
            webSocket.send(dataPackageArray[0]);

        }
    }
    else{
        if(type == 'config'){
            webSocket.send(header_type_conf + data);
        }
        else if(type == 'data'){
            webSocket.send(header_type_data + data);
        }
    }
}

function onTextAreaClicked(){
    document.getElementById('log_text_area').value = "";
}

function onBaudSelectBoxSelectionChanged(){
    var bBox = document.getElementById("baudSelector");

    if(bBox.value == "undef"){
        notifyUser("Invalid Baudrate selection");
    }
    else {
        var configString = "_C000";
        configString += 'B';
        configString += bBox.value;
        configString += 'E';

        webSocket.send(configString);
    }
}

function onDataBitSelectBoxSelectionChanged(){
    var dataBBox = document.getElementById("dataBitSelector");

    if(dataBBox.value == "undef"){
        notifyUser("Invalid Databit selection");
    }
    else {
        var configString = "_C000";
        configString += 'D';
        configString += dataBBox.value;
        configString += 'E';

        webSocket.send(configString);
    }
}

function onParitySelectBoxSelectionChanged(){
    var parBox = document.getElementById("paritySelector");

    if(parBox.value == "undef"){
        notifyUser("Invalid Parity selection");
    }
    else {
        var configString = "_C000";
        configString += 'P';
        configString += parBox.value;
        configString += 'E';

        webSocket.send(configString);
    }
}

function onStoppBitSelectBoxSelectionChanged(){
    var stpBox = document.getElementById("stoppBitSelector");

    if(stpBox.value == "undef"){
        notifyUser("Invalid Stoppbit selection");
    }
    else {
        var configString = "_C000";
        configString += 'S';
        configString += stpBox.value;
        configString += 'E';

        webSocket.send(configString);
    }
}

function readConfigString(configString){
  // syntax
  // '_Ccrq' + [br][br] + [db] + [pa] + [sb] + 'E'

    // set baudrate setting
    var str = "";
    if(configString[5] != '0'){
        str += configString[5];
    }
    str+= configString[6];
    var bIndex = parseInt(str);
    document.getElementById("baudSelector").value =
        baudValueFromBaudIndex(bIndex);

    // set databit setting
    if((configString[7] == '5')||(configString[7] == '6')||(configString[7] == '7')||(configString[7] == '8')){
        str = "";
        str += configString[7];
    }
    else {
        str = "undef";
    }
    document.getElementById("dataBitSelector").value = str;

    // set parity setting
    str = "";
    str += configString[8];
    bIndex = parseInt(str);
    document.getElementById("paritySelector").value = parityStringFromIndex(bIndex);

    // set stoppbit setting
    if((configString[9] == '1')||(configString[9] == '2')){
        str = "";
        str += configString[9];
    }
    else {
        str = "undef";
    }
    document.getElementById("stoppBitSelector").value = str;
}

function baudValueFromBaudIndex(index) {
    switch (index){
        case 0:
          return "75";
        case 1:
          return "300";
        case 2:
          return "1200";
        case 3:
          return "2400";
        case 4:
          return "4800";
        case 5:
          return "9600";
        case 6:
          return "14400";
        case 7:
          return "19200";
        case 8:
          return "28800";
        case 9:
          return "38400";
        case 10:
          return "57600";
        case 11:
          return "115200";
        case 12:
          return "230400";
        case 13:
          return "460800";
        default:
          return "undef";
      }
}

function parityStringFromIndex(index){
    switch(index){
        case 0:
            return "none";
        case 1:
            return "even";
        case 2:
            return "odd";
        default:
            return "undef";
    }
}


