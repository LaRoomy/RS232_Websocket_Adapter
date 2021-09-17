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
    document.getElementById('notificationView').innerHTML = message
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
    //webSocket.send()
    //notifyUser("This is a Message !")
}

function onReceiveButtonClicked(){

}



