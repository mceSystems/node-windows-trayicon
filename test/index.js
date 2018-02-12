const WindowsTryicon = require("..");

const myTrayApp = new WindowsTryicon({
	icon: "my-icon.ico",
	menu: [
		{
			id: "item-1-id",
			caption: "First Item"
		},
		{
			id: "item-2-id-show-balloon",
			caption: "Show Balloon"
		},
		{
			id: "item-3-id-exit",
			caption: "Exit"
		}
	]
});

myTrayApp.item((id) => {
	switch (id) {
		case "item-1-id": {
			// do something
			break;
		}
		case "item-2-id": {
			myTrayApp.balloon("Hello There!", "This is my message to you").then(() => {
				console.log("Balloon dismissed");
			})
			break;
		}
		case "item-3-id-exit": {
			myTrayApp.exit();
			break;
		}
	}
});
