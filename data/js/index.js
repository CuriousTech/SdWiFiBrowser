keepAlive=3
sdbusy=false
path='/'
pathS='/'

function sendWsVar(key,value)
{
    ws.send('{"'+key+'":\"'+value+'\"}')
}

function deleteFile(name)
{
    sendWsVar('delete', pathS+name)
    httpGetList()
}

function handleTimer()
{
    if(keepAlive)
    {
        if(--keepAlive == 0)
            alert("Connection timed out")
    }
}

function setDisk(which)
{
    sendWsVar('disk', which)
    path='/'
    pathS='/'
    httpGetList()
}

function openSocket(){
    ws=new WebSocket("ws://"+window.location.host+"/ws")
    dt=new Date()
    ws.onopen=function(evt){
        sendWsVar('time',(dt.valueOf()/1000).toFixed())
        httpGetList()
    }
    ws.onclose=function(evt){alert("Connection closed")}
    ws.onmessage=function(evt){
      console.log(evt.data)
      d=JSON.parse(evt.data)
      keepAlive=3
      switch(d.type)
      {
        case 'info':
            document.getElementById('SD').value=(+d.disk?" ":"⮞")+'SD Card '+niceBytes(+d.sdfree*1024)+' free'
            document.getElementById('SD').style=(+d.disk)?"":"color:aqua"
            document.getElementById('INT').value=(+d.disk?"⮞":" ")+'Internal '+niceBytes(+d.intfree*1024)+' free'
            document.getElementById('INT').style=(+d.disk)?"color:aqua":""
            break
        case 'alert':
            alert(d.value)
            break
        case 'filelist':
            onHttpList(d.value)
            break
      }
    }
    setInterval(handleTimer, 1000)
}

function httpGetList() {
    sdbusy = true
    sendWsVar("list", path)
}

function httpRelinquishSD() {
    sendWsVar('relinquish', 0)
}

function getContentType(filename) {
	if (filename.endsWith(".htm")) return "text/html";
	else if (filename.endsWith(".html")) return "text/html";
	else if (filename.endsWith(".css")) return "text/css";
	else if (filename.endsWith(".js")) return "application/javascript";
	else if (filename.endsWith(".json")) return "application/json";
	else if (filename.endsWith(".png")) return "image/png";
	else if (filename.endsWith(".gif")) return "image/gif";
	else if (filename.endsWith(".jpg")) return "image/jpeg";
	else if (filename.endsWith(".ico")) return "image/x-icon";
	else if (filename.endsWith(".xml")) return "text/xml";
	else if (filename.endsWith(".pdf")) return "application/x-pdf";
	else if (filename.endsWith(".zip")) return "application/x-zip";
	else if (filename.endsWith(".gz")) return "application/x-gzip";
	return "text/plain";
}

function onClickDownload(filename) {
    
    if(sdbusy) {
        alert("SD card is busy");
        return
    }
    sdbusy = true;

    document.getElementById('probar').style.display="block";

    var type = getContentType(filename);
    // let urlData = '/ids/report/exportWord' + "?startTime=" + that.report.startTime + "&endTime=" + that.report.endTime +"&type="+type
    let urlData = "/download?path=" + pathS+filename
    let xhr = new XMLHttpRequest()
    xhr.open('GET', urlData, true)
    xhr.setRequestHeader("Content-Type", type + ';charset=utf-8')
    xhr.responseType = 'blob'
    xhr.addEventListener('progress', event => {
        const percent  = ((event.loaded / event.total) * 100).toFixed(2);
        console.log(`downloaded:${percent} %`);

        var progressBar = document.getElementById('progressbar');
        if (event.lengthComputable) {
          progressBar.max = event.total
          progressBar.value = event.loaded
        }
    }, false)
    xhr.onload = function (e) {
      if (this.status == 200) {
        let blob = this.response
        let downloadElement = document.createElement('a')
        let url = window.URL.createObjectURL(blob)
        downloadElement.href = url
        downloadElement.download = filename
        downloadElement.click()
        window.URL.revokeObjectURL(url)
        sdbusy = false
        console.log("download finished")
        document.getElementById('probar').style.display="none"
        httpRelinquishSD();
      }
    };
    xhr.onerror = function (e) {
        alert(e)
        alert('Download failed!')
        document.getElementById('probar').style.display="none"
    }
    xhr.send()
}

function onClickEnter(folder) {
    
    if(sdbusy) {
        alert("SD card is busy")
        return
    }
    if(folder==".."){
        pathParts=path.split('/')
        pathParts.pop()
        path=pathParts.join('/')
        if(path=='') path='/'
    }else{
        if(path.length>1) path+='/'
        path+=folder
    }
    pathS=path
    if(pathS.length>1) pathS+='/'
    onClickUpdateList()
}

function onUploaded(evt) {
    $("div[role='progressbar']").css("width",0)
    $("div[role='progressbar']").attr('aria-valuenow',0)
    document.getElementById('probar').style.display="none"
    httpGetList()
    document.getElementById('uploadButton').disabled = false
    console.log('Upload done!')
}

function onUploadFailed(evt) {
    document.getElementById('probar').style.display="none"
    document.getElementById('uploadButton').disabled = false
    alert('Upload failed!')
}

function onUploading(evt) {
    var progressBar = document.getElementById('progressbar');
    if (evt.lengthComputable) {
      progressBar.max = evt.total
      progressBar.value = evt.loaded
    }
}

function onClickUpload() {
    if(sdbusy) {
        alert("SD card is busy");
        return
    }

    var input = document.getElementById('Choose');
    if (input.files.length === 0) {
        alert("Please choose a file first")
        return
    }
    if(input.files[0].size==0)// assume dir
    {
        sendWsVar("createdir", pathS+input.files[0].name)
        httpGetList()
        return
    }

    sdbusy = true
    document.getElementById('uploadButton').disabled = true
    document.getElementById('probar').style.display="block"

    xmlHttp = new XMLHttpRequest()
    xmlHttp.onload = onUploaded
    xmlHttp.onerror = onUploadFailed
    xmlHttp.upload.onprogress = onUploading
    var formData = new FormData()
    savePath = pathS+input.files[0].name
    formData.append('data', input.files[0], savePath)
    xmlHttp.open('POST', '/upload')
    xmlHttp.send(formData)
}

function niceBytes(n){
    const units = ['bytes', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB']
    l = 0

    while(n >= 1024 && ++l)
        n = n/1024
    return(n.toFixed(n < 10 && l > 0 ? 1 : 0) + ' ' + units[l])
}

function createFilelistItem(i,type,filename,size) {
    var data =  '<div class="media">\n' + 
                '<div class="file-index" >'+i+'</div>\n' +
                '<div class="media-body tm-bg-gray">\n' +
                    '<div class="tm-description-box">\n' +
                      '<h5 id=filename class="tm-text-blue"' + (type=="dir"?"style='color:orange'":"") +'>'+filename+'</h5>\n' +
                      (type=="dir"?'<p class="mb-0"> </br></p>':'<p class="mb-0">Size:'+size+'</p>') +
                    '</div>\n' +
                    '<div class="tm-dd-box">\n' +
                      '<input id="'+filename+'" type="button" value="Delete" class="btn tm-bg-blue tm-text-white tm-dd" onclick="{deleteFile(id)}" />'
    if(type=='dir')
        data += '<input id="'+filename+'" type="button" method="GET" value="Enter" class="btn tm-bg-blue tm-text-white tm-dd" onclick="{onClickEnter(id)}" />'
    else
        data += '<input id="'+filename+'" type="button" method="GET" value="Download" class="btn tm-bg-blue tm-text-white tm-dd" onclick="{onClickDownload(id)}" />'

    data += '</div>\n' +
            '</div>\n' +
            '</div>'
    return data
}

function onHttpList(list) {
    document.getElementById("filelistbox").innerHTML=""
    if(path.length>1)
        $("#filelistbox").append(createFilelistItem(0,"dir","..",niceBytes(0)))

    list.sort(function(a1, b1) {
      n=a1.name.toLowerCase().localeCompare(b1.name.toLowerCase())
      if(a1.type=='dir') n-=10
      if(b1.type=='dir') n+=10
      return n
    })

    for (var i = 0; i < list.length; i++) {
        // console.log(list[i].name)
        // console.log(list[i].size)
        $("#filelistbox").append(createFilelistItem(i+1,list[i].type,list[i].name,niceBytes(list[i].size)))
    }
    sdbusy=false
}

function onClickUpdateList() {
    if(sdbusy) {
        alert("SD card is busy")
        return
    }
    httpGetList()
}
