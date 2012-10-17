var dY = 0.5;
var dX = Math.sqrt(1*1 - 0.5*0.5);

function path(c, vertices) {
    c.beginPath();
    c.moveTo(vertices[0], vertices[1]);
    for (var i = 2; i < vertices.length; i += 2)
	c.lineTo(vertices[i], vertices[i + 1]);
    c.closePath();
}

function drawCube(c) {
    // top
    c.fillStyle = '#ffffff';
    path(c, [dX, 0,
	     2 * dX, dY,
	     dX, 2 * dY,
	     0, dY]);
    c.fill();

    // left
    c.fillStyle = '#555555';
    path(c, [0, dY,
	     dX, 2 * dY,
	     dX, 4 * dY,
	     0, 3 * dY]);
    c.fill();

    // right
    c.fillStyle = '#aaaaaa';
    path(c, [2 * dX, dY,
	     2 * dX, 3 * dY,
	     dX, 4 * dY,
	     dX, 2 * dY]);
    c.fill();
}

$(function() {
    var c = $('#canvas')[0].getContext('2d');
    c.scale(100, 100);
    drawCube(c);
});
