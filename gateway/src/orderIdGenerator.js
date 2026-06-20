let counter = 1;

function nextOrderId(){
    return counter++;
}

module.exports = { nextOrderId };