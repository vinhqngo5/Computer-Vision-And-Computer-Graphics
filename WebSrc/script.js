const shoot = () => {
	const bullet = document.createElement("a-sphere");
	let pos = myCamera.getAttribute("position");
	bullet.setAttribute("position", pos);
	bullet.setAttribute("velocity", getDirection(myCamera, 30));
	bullet.setAttribute("dynamic-body", true);
	bullet.setAttribute("radius", 0.5);
	bullet.setAttribute("src", "https://i.imgur.com/H8e3Vnu.png");
	myScene.appendChild(bullet);
};

document.onkeydown = (event) => {
	if (event.which == 32) {
		shoot();
	}
};
