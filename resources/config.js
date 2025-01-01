"use strict";


function dumpConfig() {
    return {
        "wifi": {
            "hostname": $id('wifi-hostname').value,
            "sta_priority": $id('sta-priority-list').getValues(),
            "access_point": {
                "enabled": $id('enable-ap').checked,
                "ssid": $id('ap-ssid').value,
                "password": $id('ap-password').value,
                "hidden": $id('ap-hidden').checked,
                "captive": $id('ap-captive').checked,
                "channel": Number($id('ap-channel').value),
                "address": $id('ap-address').value,
                "gateway": $id('ap-gateway').value,
                "subnet": $id('ap-subnet').value
            },
        },
        "channels": {
            "webMode": $id('web-mode-colorspace').value,
            "knobMode": $id('knobs-mode-colorspace').value,
            "defaultColorEnabled": $id('enable-default-color').checked,
            "defaultColor": {
                "hue": Number($id('default-color-hue').value),
                "saturation": Number($id('default-color-saturation').value),
                "value": Number($id('default-color-value').value),
                "white": Number($id('default-color-white').value)
            }
        },
        "filters": {
            "inputFilters": {
                "hue": $id('input-hue').getValues(),
                "saturation": $id('input-saturation').getValues(),
                "value": $id('input-value').getValues(),
                "lightness": $id('input-lightness').getValues(),
                "red": $id('input-red').getValues(),
                "green": $id('input-green').getValues(),
                "blue": $id('input-blue').getValues(),
                "white": $id('input-white').getValues()
            },
            "globalInputFilters": $id('input-common-filter').getValues(),
            "globalOutputFilters": $id('output-common-filter').getValues(),
            "outputFilters": {
                "red": $id('output-red').getValues(),
                "green": $id('output-green').getValues(),
                "blue": $id('output-blue').getValues(),
                "white": $id('output-white').getValues()
            }
        },
        "hardware": {
            "potentionemterConfiguration": $id('input-table').getValues(),
            "transistorConfiguration": $id('output-table').getValues(),
            "bias": {
                "down": Number($id('low-bias-input').value),
                "up": Number($id('high-bias-input').value)
            },
            "scalling": {
                "red": Number($id('red-scalling-factor').value),
                "green": Number($id('green-scalling-factor').value),
                "blue": Number($id('blue-scalling-factor').value),
                "white": Number($id('white-scalling-factor').value)
            },
            "phaseMode": Number($id('phase-mode').value),
            "knobActivateDelta": Number($id('knob-activate-input').value),
            "knobsNoisesReduction": Number($id('knob-noises-reduction').value),
            "gateLoadingTime": Number($id('gate-loading-time').value),
            "frequency": Number($id('frequency-selector').value),
            "enableColorKnob": $id('activate-color-knob').checked,
            "enableWhiteKnob": $id('activate-white-knob').checked,
            "invertOutputs": $id('invert-outputs').checked
        }
    };
}


function fillConfig(config) {
    $id('wifi-hostname').value = config.wifi.hostname;
    $id('sta-priority-list').setValues(config.wifi.sta_priority);
    $id('enable-ap').checked = config.wifi.access_point.enabled;
    $id('ap-ssid').value = config.wifi.access_point.ssid;
    $id('ap-password').value = config.wifi.access_point.password;
    $id('ap-hidden').checked = config.wifi.access_point.hidden;
    $id('ap-captive').checked = config.wifi.access_point.captive;
    $id('ap-channel').value = config.wifi.access_point.channel;
    $id('ap-address').value = config.wifi.access_point.address;
    $id('ap-gateway').value = config.wifi.access_point.gateway;
    $id('ap-subnet').value = config.wifi.access_point.subnet;

    $id('web-mode-colorspace').value = config.channels.webMode;
    $id('knobs-mode-colorspace').value = config.channels.knobMode;
    $id('enable-default-color').checked = config.channels.defaultColorEnabled;
    $id('default-color-hue').value = config.channels.defaultColor.hue;
    $id('default-color-saturation').value = config.channels.defaultColor.saturation;
    $id('default-color-value').value = config.channels.defaultColor.value;
    $id('default-color-white').value = config.channels.defaultColor.white;

    $id('input-hue').setValues(config.filters.inputFilters.hue);
    $id('input-saturation').setValues(config.filters.inputFilters.saturation);
    $id('input-value').setValues(config.filters.inputFilters.value);
    $id('input-lightness').setValues(config.filters.inputFilters.lightness);
    $id('input-red').setValues(config.filters.inputFilters.red);
    $id('input-green').setValues(config.filters.inputFilters.green);
    $id('input-blue').setValues(config.filters.inputFilters.blue);
    $id('input-white').setValues(config.filters.inputFilters.white);
    $id('input-common-filter').setValues(config.filters.globalInputFilters);
    $id('output-common-filter').setValues(config.filters.globalOutputFilters);
    $id('output-red').setValues(config.filters.outputFilters.red);
    $id('output-green').setValues(config.filters.outputFilters.green);
    $id('output-blue').setValues(config.filters.outputFilters.blue);
    $id('output-white').setValues(config.filters.outputFilters.white);

    $id('input-table').setValues(config.hardware.potentionemterConfiguration);
    $id('output-table').setValues(config.hardware.transistorConfiguration);

    $id('low-bias-input').value = config.hardware.bias.down;
    $id('high-bias-input').value = config.hardware.bias.up;

    $id('red-scalling-factor').value = config.hardware.scalling.red;
    $id('green-scalling-factor').value = config.hardware.scalling.green;
    $id('blue-scalling-factor').value = config.hardware.scalling.blue;
    $id('white-scalling-factor').value = config.hardware.scalling.white;

    $id('phase-mode').value = config.hardware.phaseMode;
    $id('knob-activate-input').value = config.hardware.knobActivateDelta;
    $id('knob-noises-reduction').value = config.hardware.knobsNoisesReduction;
    $id('gate-loading-time').value = config.hardware.gateLoadingTime;
    $id('frequency-selector').value = config.hardware.frequency;
    $id('activate-color-knob').checked = config.hardware.enableColorKnob;
    $id('activate-white-knob').checked = config.hardware.enableWhiteKnob;
    $id('invert-outputs').checked = config.hardware.invertOutputs;
}


$id('save-settings-btn').addEventListener('click', ()=>{config = dumpConfig(); saveConfig();});
$id('revert-settings-btn').addEventListener('click', ()=>{configPromise = refreshConfig().then(()=>fillConfig(config));});
$id('default-settings-btn').addEventListener('click', async ()=>{
    if (confirm("All settings "+"(wifi credentials, filters, etc)"+" will be reverted to default values."+" "+"Are you sure you want to continue?")) {
        await refreshConfig(true);
        await saveConfig();
    }
});
$id('export-settings-btn').addEventListener('click', exportConfigToJSON);
$id('import-settings-btn').addEventListener('click', async ()=>{
    try {
        const newConfig = await importConfigFromFile();
        if (newConfig === null) return;
        fillConfig(newConfig);
        config = newConfig;
        alert("Configuration imported."+" "+"Now you can save them.");
    } catch {
        fillConfig(config);
        alert('Error ocurred.'+' Invalid configuration file.');
    }
});


function refreshNetworks() {
    return fetch('/refresh_networks');
}


function openAccessPoint() {
    fetch('/open_access_point');
}


function reconnect() {
    fetch('/reconnect');
}


function deleteCert() {
    fetch('/delete_cert');
}


function restart() {
    fetch('/restart');
}


async function getTailoredScalling() {
    const response = await fetch('/get_tailored_scalling');
    const data = await response.json();
    $id('red-scalling-factor').value = data[0];
    $id('green-scalling-factor').value = data[1];
    $id('blue-scalling-factor').value = data[2];
    $id('white-scalling-factor').value = data[3];
}


function setDefaultsBlack() {
    $id('default-color-hue').value = 0;
    $id('default-color-saturation').value = 0;
    $id('default-color-value').value = 0;
    $id('default-color-white').value = 0;
}


function setDefaultsWhite() {
    $id('default-color-hue').value = 0;
    $id('default-color-saturation').value = 0;
    $id('default-color-value').value = 1;
    $id('default-color-white').value = 1;
}


async function setDefaultsCurrent() {
    const response = await fetch('/color.json?colorspace=hsv');
    const data = await response.json();
    $id('default-color-hue').value = data[0];
    $id('default-color-saturation').value = data[1];
    $id('default-color-value').value = data[2];
    $id('default-color-white').value = data[3];
}


updateClientApp();
