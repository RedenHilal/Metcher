import {spawn} from "node:child_process"
import axios from "axios"

const arg = process.argv
let choice = 0;

if (arg.length>2){
    if (arg[2][0]==="r") choice = 1;
}

function clearLog(){
    process.stdout.write('\x1Bc');
}

(async function main(){
    const mpd = spawn("mpc", ["idleloop","player"]);

    mpd.stdout.on("data",data=>{

        const status = spawn("mpc",["status"]);
        status.stdout.on("data", async song=>{
            try{
            
                const songName = song.toString().split("\n")[0]
                const regex = /\s+/g
                const removeSpace = songName.replace(regex, "")
                console.log(`"http://localhost:5555/search?q=${removeSpace}"`)
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
                
                clearLog()
                console.log(captured)
                console.log(lyricBetter)
            }
            catch(err){
                console.log("Cant found song. either server error or bad metadata")
            }
        })
    })
})()