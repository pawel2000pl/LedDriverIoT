$id('update-btn').addEventListener('click', async function(e) {
    e.preventDefault();
    const fileInput = $id('update-file');
    $id('update-btn').disabled = true;

    fetch('/update', {
        method: 'POST',
        body: await fileInput.files[0].bytes(),
        headers: {
            'Accept': 'application/json'
        }
    })
    .then(response => {
        if (!response.ok) {
            throw new Error('Device response was not ok');
        }
        alert('Success! Wait until controller connect to your network agan and then refresh this page.');
    })
    .catch(error => {
        alert("There was a problem with the network connection.\nBut it is possible that the device just restarted before it sent response that everything is ok,\nso refresh this page and check if version has changed.");
    }).finally(()=>{
        $id('update-btn').disabled = false;
    });
});


fetchVersion().then(([version, hardware, resources])=>{
    $id('version-span').textContent = "Current version: " + version;
    $id('hardware-span').textContent = "Hardware: " + hardware;
    $id('resources-span').textContent = "Resources: " + resources;
});
