
$id('update-form').addEventListener('submit', function(e) {
    e.preventDefault();
    var form = $id('update-form');
    var data = new FormData(form);

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
    });
});


fetch('/build_info.json').then(async (response)=>{
    const data = await response.json();
    $id('version-p').textContent = "Current version: " + data.message;
});
