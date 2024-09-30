"use strict";

const favoritesDiv = $id('favorites-div');

function favoriteColorFactory(paramColor=null) {
    var getFunction = ()=>{return {red:0, green:0, blue:0, white:0};};
    const div = $new('div');
    div.textContent = 'Please wait...';
    div.getValue = ()=>{return getFunction();};
    
    const setColor = async (color)=>{
        const filteredQueryParams = new URLSearchParams();
        filteredQueryParams.append('red', color.red);
        filteredQueryParams.append('green', color.green);
        filteredQueryParams.append('blue', color.blue);
        filteredQueryParams.append('white', color.white);
        const filteredResponse = await fetch('/filtered_color.json?'+filteredQueryParams.toString());
        const filteredColors = await filteredResponse.json();
        const filteredRGB = converters[config.channels.webMode](...filteredColors);

        div.textContent = '';
        const colorBar = $new('div');
        colorBar.className = 'color-bar';
        colorBar.style.backgroundColor = `rgb(${255*filteredRGB[0]},${255*filteredRGB[1]},${255*filteredRGB[2]})`;
        colorBar.style.color = (filteredRGB[0] * 0.2989 + 0.5870 * filteredRGB[1] + 0.1140 * filteredRGB[2] > 0.5) ? 'black' : 'white';
        div.appendChild(colorBar);
        const applyBtn = $new('button');
        applyBtn.textContent = 'Apply';
        applyBtn.onclick = ()=>{
            fetch('/color.json', {
                method: 'POST',
                headers: {"Content-Type": "application/json"},
                body: JSON.stringify(color)
            });
        };
        div.appendChild(applyBtn);
        const bookmarkBtn = $new('button');
        bookmarkBtn.textContent = 'Fast URL';
        bookmarkBtn.onclick = ()=>{
            const params = new URLSearchParams();
            params.append('red', color.red);
            params.append('green', color.green);
            params.append('blue', color.blue);
            params.append('white', color.white);
            window.open('/favorite_color.html?'+params.toString(), '_blank').focus();
        };
        div.appendChild(bookmarkBtn);
        getFunction = ()=>{return color;};
    };
    if (paramColor !== null)
        setColor(paramColor);
    else
        fetch('/color.json').then(async (response)=>{
            const color = await response.json();
            await setColor(color);
        });

    return div;
}


configPromise.then(()=>{
    componentList(favoritesDiv, favoriteColorFactory);
    config.favorites.forEach((color)=>favoritesDiv.addElement(color));
    const saveBtn = $new('button');
    saveBtn.textContent = 'Save changes';
    saveBtn.onclick = ()=>{
        config.favorites = favoritesDiv.getValues();
        saveConfig();
    };
    favoritesDiv.appendChild(saveBtn);
});
