$id('update-btn').addEventListener('click', async function(e) {
    e.preventDefault();
    const fileInput = $id('update-file');
    $id('update-btn').disabled = true;
    const currentVersion = await fetchVersion();

    const successAlert = ()=>alert('Success! Wait until the controller connect to your network again and then refresh this page.');
    const probablySuccessAlert = ()=>alert('There was a problem with the network connection. But it is possible that the device just restarted before it sent a response that everything is ok, so refresh this page and check if the version has changed.');
    const errorAlert = ()=>alert('Update error: the response was not "ok"');

    fetch('/update', {
        method: 'POST',
        body: await fileInput.files[0].bytes(),
        headers: {
            'Accept': 'application/json'
        }
    })
    .then(response => {
        if (!response.ok) {
            errorAlert();
        } else {
            successAlert();
        }
    })
    .catch(async (error)=>{
        const newVersion = await fetchVersion();
        if (currentVersion[0] != newVersion[0])
            successAlert();
        else
            probablySuccessAlert();
        
    }).finally(()=>{
        $id('update-btn').disabled = false;
    });
});


fetchVersion().then(([version, hardware, resources])=>{
    $id('version-span').textContent = "Current version: " + version;
    $id('hardware-span').textContent = "Hardware: " + hardware;
    $id('resources-span').textContent = "Resources: " + resources;
});
