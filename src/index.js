import {spawn} from "node:child_process"
import axios from "axios"
import net from "net"

let socket = null;

const server = net.createServer((sockets)=>{
    socket = sockets;

    sockets.on("end",()=>{
        console.log("client Disconnected");
        socket = null;
    })
})

server.listen(7700, ()=>{
    console.log("Server Listening on port 7700")
});
const PORT = 7700

const arg = process.argv
let choice = 0;

if (arg.length>2){
    if (arg[2][0]==="r") choice = 1;
}


(async function main(){
    
    fetchLyric();
    
})()

function encodeMessage(char16){
    const encoder = new TextEncoder()
    const char8 = encoder.encode(char16)
    return char8
}


async function fetchLyric(){
    const mpd = spawn("mpc", ["idleloop","player"]);
    mpd.stdout.on("data",data=>{

    const status = spawn("mpc",["status"]);

    status.stdout.on("data", async song=>{
        try{
        
            const songName = song.toString().split("\n")[0]
            const regex = /\s+/g
            const removeSpace = songName.replace(regex, "")
            //console.log(`"http://localhost:5555/search?q=${removeSpace}"`)
            const searchRespone = await axios.get(`http://localhost:5555/search?q=${songName}`
                ,{responseType:"text"}
            )

            let captureLink = choice? /href="([^"]+romanized[^"]+)">/:/href="(?!\/Genius)([^"]*lyrics)+"/
            if(!captureLink.test(searchRespone.data)){
                captureLink = choice? /href="(?!\/Genius)([^"]*lyrics)+"/:/href="([^"]+romanized[^"]+)">/
            }
            const captured = searchRespone.data.match(captureLink)[1]
            const lyricRaw = await axios.get(`http://localhost:5555/${captured}`)
            const lyric = lyricRaw.data.replace(/<\/?[^>]+(>|$)/g, '\n').trim();
            const lyricBetter = lyric.replace(/\n+/g, "\n")
            
            const encoded = encodeMessage(lyricBetter)
            if(!socket) throw {message:"",code : 3};
            if(!socket.write(encoded)){
                throw {message : "",code : 2}
            }
            console.log(`Sent ${songName}`)
        }
        catch(err){
            if (err.code === 2){
                console.log("Failed send")
            } else if(err.code == 3){

            }
            else 
            console.log("Cant found song. either server error or bad metadata" + err.code);
        }
    })

    })
}