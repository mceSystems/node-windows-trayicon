const WindowsTryicon = require("..");

const myTrayApp = new WindowsTryicon({
	title: "Trayicon Test",
	icon: "c:\\tmp\\icon.ico",
	menu: [
		{
			id: "item-1-id",
			caption: "First Item"
		},
		{
			id: "item-2-id",
			caption: "Second Item"
		},
		{
			id: "item-3-id-exit",
			caption: "Exit"
		}
	]
});

myTrayApp.item((id) => {
	console.log(`Menu id selected=${id}`);
	switch (id) {
		case "item-1-id": {
			console.log("First item selected...");
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
			process.exit(0)
			break;
		}
	}
});

process.stdin.resume()