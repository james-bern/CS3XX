// TODOLATER: resizing


var worldHeight = 128.0;

// Create an empty scene
var scene = new THREE.Scene();

// Create a basic perspective camera
var aspect = window.innerWidth / window.innerHeight;
var camera = new THREE.PerspectiveCamera(90, aspect, 0.1, 9999);
camera.position.z = worldHeight / 2;

var renderer = new THREE.WebGLRenderer({antialias:false});
renderer.setClearColor("#FFFFFF");
renderer.setSize( window.innerWidth, window.innerHeight );
document.body.appendChild( renderer.domElement );

var gh = new THREE.GridHelper(64, 64, "black", "black");
gh.rotation.x = Math.PI * 0.5;
scene.add(gh);

var canvas = document.createElement("canvas");
var map = new THREE.CanvasTexture(canvas);
canvas.width = 1024;
canvas.height = 1024;
var ctx = canvas.getContext("2d");
ctx.fillStyle = "black";
ctx.fillRect(0, 0, canvas.width, canvas.height);

function drawRectangle(x, y, w, h, color) {
    ctx.fillStyle = color;
    ctx.fillRect(x, y, w, h);
}


var plane = new THREE.Mesh(new THREE.PlaneBufferGeometry(worldHeight, worldHeight), new THREE.MeshBasicMaterial({
    color: "white",
    map: map
}));
scene.add(plane);


var t = 0.0;
var render = function () {
    requestAnimationFrame( render );
    renderer.render(scene, camera);
    t += 1;
    ctx.fillStyle = "aqua";
    ctx.fillRect(10 + t, 10, 30, 30);
    ctx.fillStyle = "magenta";
    ctx.fillRect(00, 60, 60, 60);
    ctx.fillStyle = "yellow";
    ctx.fillRect(30, 20, 80, 80);
    ctx.font = "50px Arial";
    ctx.fillStyle = "white";
    ctx.fillText("Hello World",10,80);
    map.needsUpdate = true;
};

render();
