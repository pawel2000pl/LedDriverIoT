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


fetchVersion().then(([version, hardware, resources])=>{
    $id('version-span').textContent = "Current version: " + version;
    $id('hardware-span').textContent = "Hardware: " + hardware;
    $id('resources-span').textContent = "Resources: " + resources;
});
