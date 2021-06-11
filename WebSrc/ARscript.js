const GAME = {
	MAX_VELOCITY: 20,
	DEFAULT_VELOCITY: 10,
};

// let nextLevel = "index.html";

const shoot = (velocity = GAME.DEFAULT_VELOCITY) => {
	const bullet = document.createElement("a-sphere");
	let pos = myCamera.getAttribute("position");
	bullet.setAttribute("position", pos);
	bullet.setAttribute("velocity", getDirection(myCamera, velocity));
	bullet.setAttribute("dynamic-body", true);
	bullet.setAttribute("radius", 0.2);
	bullet.setAttribute("src", "https://i.imgur.com/H8e3Vnu.png");
	myScene.appendChild(bullet);
	bullet.addEventListener("collide", shootCollided);
};

const shootCollided = (event) => {
	if (event.detail.body.el.id === "floor") {
		event.detail.target.el.removeEventListener("collide", shootCollided);
	} else if (event.detail.body.el.className === "target") {
		console.log("Hit the target");
		marker.removeChild(event.detail.body.el);
	} else if (event.detail.body.el.className === "text") {
		let textWrapper = document.querySelector("#text-wrapper");
		myScene.removeChild(textWrapper);
	}
	if (document.querySelectorAll(".target").length === 0) {
		console.log("You win");
		location.href = nextLevel;
	}
};

const shootInDesktop = (el, callback) => {
	let touchsurface = el,
		elapsedTime,
		startTime,
		handlePress = callback || function (swipeDir) {};

	touchsurface.addEventListener(
		"keydown",
		function (e) {
			if (e.repeat) {
				return; // Event fired when press too long
			}
			startTime = new Date().getTime(); // record time when finger first makes contact with surface
			e.preventDefault();
		},
		false
	);

	touchsurface.addEventListener(
		"keyup",
		function (e) {
			elapsedTime = new Date().getTime() - startTime; // get time elapsed
			handlePress(e, Math.min(elapsedTime / 20, GAME.MAX_VELOCITY));
			e.preventDefault();
		},
		false
	);
};

const swipeDetect = (el, callback) => {
	var touchsurface = el,
		swipeDir,
		startX,
		startY,
		distX,
		distY,
		threshold = 30, //required min distance traveled to be considered swipe
		restraint = 100, // maximum distance allowed at the same time in perpendicular direction
		allowedTime = 300, // maximum time allowed to travel that distance
		elapsedTime,
		startTime,
		handleswipe = callback || ((swipeDir, velocity) => {});

	touchsurface.addEventListener(
		"touchstart",
		function (e) {
			var touchobj = e.changedTouches[0];
			swipeDir = "none";
			dist = 0;
			startX = touchobj.pageX;
			startY = touchobj.pageY;
			startTime = new Date().getTime(); // record time when finger first makes contact with surface
			e.preventDefault();
		},
		false
	);

	touchsurface.addEventListener(
		"touchmove",
		function (e) {
			e.preventDefault(); // prevent scrolling when inside DIV
		},
		false
	);

	touchsurface.addEventListener(
		"touchend",
		function (e) {
			var touchobj = e.changedTouches[0];
			distX = touchobj.pageX - startX; // get horizontal dist traveled by finger while in contact with surface
			distY = touchobj.pageY - startY; // get vertical dist traveled by finger while in contact with surface
			elapsedTime = new Date().getTime() - startTime; // get time elapsed
			if (elapsedTime <= allowedTime) {
				// first condition for awipe met
				if (Math.abs(distX) >= threshold && Math.abs(distY) <= restraint) {
					// 2nd condition for horizontal swipe met
					swipeDir = distX < 0 ? "left" : "right"; // if dist traveled is negative, it indicates left swipe
				} else if (
					Math.abs(distY) >= threshold &&
					Math.abs(distX) <= restraint
				) {
					// 2nd condition for vertical swipe met
					swipeDir = distY < 0 ? "up" : "down"; // if dist traveled is negative, it indicates up swipe
				}
			}
			handleswipe(swipeDir, Math.min(Math.abs(distY / 8), GAME.MAX_VELOCITY));
			e.preventDefault();
		},
		false
	);
};

const getTextPosition = (camera) => {
	var y = camera.getAttribute("rotation").y + 90;
	var x = camera.getAttribute("rotation").x;

	var moveX = Math.cos((y / 360) * (Math.PI * 2));
	var moveY = Math.sin((x / 360) * (Math.PI * 2));
	var moveZ = Math.sin((y / 360) * (Math.PI * 2));
	var moveXRatio =
		Math.pow(moveX, 2) / (Math.pow(moveX, 2) + Math.pow(moveZ, 2));
	var moveZRatio =
		Math.pow(moveZ, 2) / (Math.pow(moveX, 2) + Math.pow(moveZ, 2));

	if (moveX <= 0) {
		moveX = -Math.sqrt((1 - Math.pow(moveY, 2)) * moveXRatio);
	} else {
		moveX = Math.sqrt((1 - Math.pow(moveY, 2)) * moveXRatio);
	}

	return { x: moveX, y: moveY };
};

shootInDesktop(document, function (event, velocity) {
	if (event.which == 32) shoot(velocity);
});
swipeDetect(document, function (swipeDir, velocity) {
	if (swipeDir == "up") shoot(velocity);
});
