//alert("This alert box was called with the onload event");


var gateway = `ws://${window.location.hostname}/ws`;

var webSocket;

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

    // temp ?!
    notifyUser(event.data);

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

    if(length > 500){
        // if the size of data is larger than 500bytes, it must be segmented
        var index = 0;
        var lengthCounter = 0;
        var segmentNumber = 0;
        var dataToSend = "";

        while(index < length){
            
            // max package size reached -> send it
            if(lengthCounter >= 500){
                // first build a simple header for the transmission
                var segmentHeaderString = ""

                // what if this is the last package???

                if(segmentNumber == 0){
                    // this is the first package, apply the appropriate header entry
                    if(type == 'config'){
                        segmentHeaderString = "sC";
                    }
                    else if(type == 'data'){
                        segmentHeaderString = "sD"
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
                            segmentHeaderString = "eD"
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
                            segmentHeaderString = "xD"
                        }
                        else {
                            segmentHeaderString = "xU";
                        }
                    }
                }

                if(segmentNumber < 10){
                    segmentHeaderString += ("00" + segmentNumber);
                }
                else if(segmentNumber < 100){
                    segmentHeaderString += ("0" + segmentNumber);
                }
                else {
                    if(segmentNumber > 999){
                        // error
                        notifyUser("Error: Datasize to large (999). Aborting transmission.");
                    }
                    else {
                        segmentHeaderString += segmentNumber;
                    }
                }

                // TEMP!!!
                notifyUser("Formatted Header: " + segmentHeaderString);
                
                webSocket.send(segmentHeaderString + dataToSend);
                                
                dataToSend = "";
                lengthCounter = 0;
                segmentNumber++;
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
                sheaderStr = "eD"
            }
            else {
                sheaderStr = "eU";
            }

            if(segmentNumber < 10){
                sheaderStr += ("00" + segmentNumber);
            }
            else if(segmentNumber < 100){
                sheaderStr += ("0" + segmentNumber);
            }
            else {
                if(segmentNumber > 999){
                    // error
                    notifyUser("Error: Datasize to large (999). Aborting transmission.");
                }
                else {
                    sheaderStr += segmentNumber;
                }
            }
            // TEMP!!!
            notifyUser("Formatted Header: " + sheaderStr);

            webSocket.send(sheaderStr + dataToSend);
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



