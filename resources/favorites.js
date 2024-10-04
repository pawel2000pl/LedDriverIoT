"use strict";

const favoritesDiv = $id('favorites-div');

function favoriteColorFactory(paramColor=null, white=false) {
    var getFunction = ()=>{return "000000000";};
    const div = $new('div');
    div.textContent = 'Please wait...';
    div.getValue = ()=>{return getFunction();};
    
    const setColor = async (colorData)=>{        
        const filteredColors = colorData.color;
        const filteredRGB = converters[config.channels.webMode](...filteredColors);

        div.textContent = '';
        const colorBar = $new('div');
        colorBar.className = 'color-bar';
        colorBar.style.backgroundColor = `rgb(${255*filteredRGB[0]},${255*filteredRGB[1]},${255*filteredRGB[2]})`;
        colorBar.style.color = (filteredRGB[0] * 0.2989 + 0.5870 * filteredRGB[1] + 0.1140 * filteredRGB[2] > 0.5) ? 'black' : 'white';
        div.appendChild(colorBar);
        const applyBtn = $new('button');
        applyBtn.textContent = 'Apply';
        applyBtn.onclick = ()=>fetch('/apply_favorite?code='+colorData.code);
        div.appendChild(applyBtn);
        const bookmarkBtn = $new('button');
        bookmarkBtn.textContent = 'Fast URL';
        bookmarkBtn.onclick = ()=>{
            const params = new URLSearchParams();
            params.append('code', colorData.code);
            window.open('/favorite_color.html?'+params.toString(), '_blank').focus();
        };
        div.appendChild(bookmarkBtn);
        getFunction = ()=>{return colorData.code;};
    };

    if (paramColor !== null)
        setColor(paramColor);
    else
        fetch('/new_favorite?white='+(white?'1':'0')).then(async (response)=>{
            const color = await response.json();
            await setColor(color);
        });

    return div;
}

const favoritePromise = (async ()=>{
    const response = await fetch('get_favorites');
    const data = await response.json();
    return data;
})();

Promise.all([configPromise, favoritePromise]).then(([config, favorites])=>{
    componentList(favoritesDiv, favoriteColorFactory);
    favorites.forEach((colorData)=>favoritesDiv.addElement(colorData));
    const addWhiteBtn = $new('button');
    addWhiteBtn.textContent = 'Add with white';
    addWhiteBtn.onclick = ()=>{
        favoritesDiv.addElement(null, true);
    };
    const saveBtn = $new('button');
    saveBtn.textContent = 'Save changes';
    saveBtn.onclick = ()=>fetch('/save_favorites', {
            method: 'POST',
            headers: {"Content-Type": "application/json"},
            body: JSON.stringify(favoritesDiv.getValues())
        });
    if (config.hardware.enbleWhiteKnob)
        favoritesDiv.appendChild(addWhiteBtn);
    favoritesDiv.appendChild(saveBtn);
});
