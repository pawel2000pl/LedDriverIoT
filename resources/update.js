
$id('update-form').addEventListener('submit', function(e) {
    e.preventDefault();
    var form = $id('update-form');
    var data = new FormData(form);
    $id('update-btn').disabled = true;

    fetch('/update', {
        method: 'POST',
        body: data,
        headers: {
            'Accept': 'application/json'
        },
        processData: false,
        contentType: false,
        xhr: function() {
            var xhr = new XMLHttpRequest();
            xhr.upload.addEventListener('progress', function(evt) {
                if (evt.lengthComputable) {
                    var per = evt.loaded / evt.total;
                    document.getElementById('prg').innerHTML = 'progress: ' + Math.round(per * 100) + '%';
                }
            }, false);
            return xhr;
        }
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Network response was not ok');
        }
        alert('Success! Wait until controller connect to your network agan and then refresh this page.');
    })
    .catch(error => {
        alert('There was a problem with the fetch operation: ' + error.toString());
    }).finally(()=>{
        $id('update-btn').disabled = false;
    });
});


fetchVersion().then(([version, hardware])=>{
    $id('version-span').textContent = "Current version: " + version;
    $id('hardware-span').textContent = "Hardware: " + hardware;
});
