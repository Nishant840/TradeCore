let counter = 1;

function nextRequestId(){
    return `req-${counter++}`;
}

module.exports = { nextRequestId };