"use strict";


Array.from(document.getElementsByClassName('link-button')).forEach((button)=>{
    const link = button.attributes['link'] ?? './#';
    button.addEventListener('click', ()=>{
        window.open(link);
    });
    button.addEventListener('auxclick', (event)=>{
        if (event.button==1)
            window.open(link, '_blank');
    });
});


const $id = (id)=>document.getElementById(id);
const $new = (...args)=>document.createElement(...args);

