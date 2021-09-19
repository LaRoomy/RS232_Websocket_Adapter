//alert("This alert box was called with the onload event");


var gateway = `ws://${window.location.hostname}/ws`;

var webSocket;

var dataPackageArray = [];

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
        }
        else if((event.data[2] == 'u')&&(event.data[3] == 'm')&&(event.data[4] == 'g')){
            // display message to user
            var msg = ""
            for(var i = 5; i < event.data.length; i++){
                msg += event.data[i];
            }
            notifyUser(msg);
        }
    }

    // temp ?!
    //notifyUser(event.data);

}

function notifyUser(message){

    var tArea = document.getElementById("log_text_area");

    document.getElementById("log_text_area").value += ('\n' + message);

    tArea.scrollTop = tArea.scrollHeight;

}

function initControls(){
    document.getElementById('sendButton').addEventListener('click', onSendButtonClicked);
    document.getElementById('receiveButton').addEventListener('click', onReceiveButtonClicked);
}

function onLoad(event){
    initWebSocket();
    initControls();
}

window.addEventListener('load', onLoad);

function onSendButtonClicked(){

    var tArea = document.getElementById("nc_content_text_area");
    var data = tArea.value;
    var length = tArea.value.length;

    notifyUser("Sending data. Size: " + length);

    //notifyUser("Send was clicked");


    sendData('data', data, length);

    //webSocket.send(data);


    //notifyUser(data);

    //notifyUser("This is a Message !")
}

function onReceiveButtonClicked(){
    webSocket.send("Receive");
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

                // TEMP!!!
                //notifyUser("Formatted Header: " + segmentHeaderString);
                
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
            // TEMP!!!
            //notifyUser("Formatted Header: " + sheaderStr);

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



