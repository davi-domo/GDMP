
let dataX = [];
let dataXjson = [];
let c = document.querySelector("canvas");
let date_stat;

function maj_stat(name_sonde, type_sonde, day = 'null') {
    let url = "./stat/";
    let ajaxRequest = new XMLHttpRequest();
    ajaxRequest.onreadystatechange = function () {
        if (ajaxRequest.readyState == 4) {
            //si la requete est complete
            if (ajaxRequest.status == 200) {// on affiche les historique
                if (day == "all") {
                    let json = JSON.parse(ajaxRequest.response);
                    date_stat = json.length + 1;
                    for (i = 0; i < json.length; i++) {
                        document.querySelector('#select_stat').options[i + 1] = new Option(json[i].date, i + 1);
                        dataXjson[i + 1] = new Array(json[i].stat.length)
                        for (j = 0; j < json[i].stat.length; j++) {
                            dataXjson[i + 1][j] = (json[i].stat[j] != null) ? json[i].stat[j] * 10 : json[i].stat[j];
                        }
                    }
                }
                if (day == "null") {
                    let json = JSON.parse(ajaxRequest.response);
                    document.querySelector('#select_stat').options[0] = new Option('Aujourd\'hui', 0);
                    dataXjson[0] = new Array(json[0].stat.length)
                    for (j = 0; j < json[0].stat.length; j++) {
                        dataXjson[0][j] = (json[0].stat[j] != null) ? json[0].stat[j] * 10 : json[0].stat[j];
                    }
                    go_graph(0);
                }
            }
        }
    }

    if (name_sonde != null && day == "all") { // on charge les stat du spiffs
        url += name_sonde + ".json";

    }
    else if (name_sonde != null && day == "del")
    {
        url += "?sonde=" + name_sonde + "&type=" + type_sonde+"&action=del"
    }
    else {// sinon on affiche seullement les stat du jour
        url += "?sonde=" + name_sonde + "&type=" + type_sonde;
    }
    ajaxRequest.open('GET', url);
    ajaxRequest.send();
}


function select(sens) {
    let sel = document.querySelector('#select_stat');
    let now_select = sel.value;
    let new_select;

    if (sens == 0 && now_select < parseInt(date_stat) - 1) {
        new_select = parseInt(now_select) + 1;
        sel.value = new_select;
        go_graph(new_select)
    }
    if (sens == 1 && now_select >= 1) {
        new_select = parseInt(now_select) - 1;
        sel.value = new_select;
        go_graph(new_select)
    }

}
function go_graph(index) {
    let heure = new Date().getHours();


    let h = c.height;
    let w = c.width;
    let ctx = c.getContext('2d');
    ctx.clearRect(0, 0, c.width, c.height);
    let dataY = [];
    dataX = dataXjson[index];
    for (i = 0; i < 24; i++) {
        dataY[i] = i;
    }
    //dataX ;
    // un = Math.round(Math.max(...dataX)/15)
    let un = Math.round((Math.max(...dataX) - Math.min(...dataX)) / 10);
    ys = (w - 40) / dataY.length
    dataT = []


    chartLine();
    data();
    draw();
    pointes();
    texte();
    function chartLine() {
        ctx.strokeStyle = "rgb(255,255,255)";
        /*ctx.beginPath()
        ctx.moveTo(60,0)
        ctx.lineTo(60,h-30)
        ctx.stroke()*/

        ctx.beginPath()
        ctx.moveTo(w, h - 30)
        ctx.lineTo(10, h - 30)
        ctx.stroke()
    }

    function draw() {
        //ctx.save()
        ctx.strokeStyle = "#03a9f4"
        ctx.lineWidth = 2
        ctx.beginPath()
        //ctx.lineJoin = "round";
        y = 10
        height = h - 30
        line = 30
        start = 30
        // ctx.moveTo(60, h-30);
        for (data of dataX) {
            max = Math.max(...dataX),
                test = 30;
            while (max > data) {
                max = max - 1
                test += line / un
            }
            ctx.lineTo(30 + y, test)
            x = 30
            y += ys
        }
        ctx.stroke()
        ctx.restore()
    }
    function pointes() {
        y = 10
        height = h - 30
        line = 30
        start = 30
        count = 0

        min_color = Math.min(...dataX)
        max_color = Math.max(...dataX)
        for (data of dataX) {
            ctx.fillStyle = map_color(data, min_color, max_color);
            max = Math.max(...dataX),
                test = 30;
            while (max > data) {
                max = max - 1
                test += line / un
            }
            if (count === heure) {

                circle(30 + y, test, 7)
            }
            else {
                circle(30 + y, test)
            }
            dataT.push({ data: Math.round(test) + "," + Math.round(30 + y) + "," + Math.round(data) })
            x = 30
            y += ys
            count++
        }
        ctx.stroke()
    }
    function data() {

        ctx.font = "12px Arial";
        ctx.fillStyle = "rgb(255,255,255)";
        ctx.fillText('Heure', 1, h - 10);
        y = 40
        x = 30
        n = Math.max(...dataX)
        for (ydata of dataY) {
            ctx.fillText(ydata, y, h - 10);
            y += ys
        }
        /*while(x < h-30){
            ctx.font = "11px Arial";
            ctx.fillText(n/10, 0,x+5);
            n = n -un
            x += 30
        }*/

    }

    function circle(x, y, d = 5) {
        ctx.beginPath();
        ctx.arc(x, y, d, 0, 2 * Math.PI);
        ctx.fill()
    }
    function texte() {
        ctx.font = "12px Arial";
        y = -30
        height = h - 30
        line = 30
        start = 30

        min_color = Math.min(...dataX)
        max_color = Math.max(...dataX)
        for (data of dataX) {
            max = Math.max(...dataX),
                test = 30;
            while (max > data) {
                max = max - 1
                test += line / un

            }
            ctx.fillStyle = map_color(data, min_color, max_color);
            ctx.fillText(data / 10, y + 60, test - 10);
            //ctx.stroke()
            //dataT.push({ data : test + "," + 30 + y +","+data})
            x = 300
            y += ys
        }
    }
    function map_color(value, low1, high1, low2 = 0, high2 = 255) {
        let result = Math.round(low2 + (high2 - low2) * (value - low1) / (high1 - low1));
        let ret = "rgb(" + result + ",0," + (255 - result) + ")";
        return ret;
    }
}