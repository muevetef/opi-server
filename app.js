
const app = require('express')();
const http = require('http').Server(app);
const io = require('socket.io')(http);
const zlib = require('zlib')

const clients = [];

app.get('/', (req, res) => {
    res.sendFile(__dirname + '/index.html');
  });


io.on('connection', (socket) => {
  var clientIp = socket.request.connection.remoteAddress;

  console.log(clientIp);
    socket.on('storeClientInfo', data => {
        const client = {
            name: data.name,
            id: socket.id
        }
        clients.push(client)
        console.log("conected new "+data.name);
        if(client.name === 'web'){
            socket.join("web_room")
        }
        console.log(clients)
    })

    socket.on('disconnect', data => {
        for(let i=0; i < clients.length; ++i){
            const c = clients[i];
            if(c.id === socket.id){
                clients.splice(i,1)
                break
            }
        }
        console.log(clients)
    })
   
    socket.on('stream', (data) =>{       
        var decmp = zlib.inflateSync(data)
        var ret = Buffer.from(decmp, 'base64').toString() // from buffer to base64 string   
        //socket.broadcast.emit('frame', ret)   
        socket.broadcast.to("web_room").emit('frame', ret)
    })     
    let sendOpen = false     
    socket.on('barcode', (data)=>{
        socket.broadcast.to("web_room").emit('barcode', data)
        if(data === "b'http://en.m.wikipedia.org'"){
           
            if(!sendOpen){
               socket.broadcast.emit('open', 1)
               sendOpen = true
               setTimeout(() => {
                sendOpen = false
               }, 2000);
            }
        }
    })
    socket.on('event_name', (data)=>{
        //socket.broadcast.emit('barcode', data)

        console.log(data)
    })
    socket.on('open', (data)=>{
        socket.broadcast.emit('open', data)
        
        console.log("open: "+data)
    })
})

module.exports = http
