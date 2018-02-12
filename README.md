# windows-trayicon
Native addon to add a windows tray icon with menu, built on windows-native libraries (no .NET dependency)  

# Installation
```
npm install --save windows-trayicon
```

# Usage
```
const WindowsTryicon = require("windows-trayicon");

const myTrayApp = new WindowsTryicon({
	icon: "my-icon.ico",
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
	switch(id){
		case "item-1-id": {
			// do something
			break;
		}
		case "item-2-id": {
			// do something else...
			break;
		}
		case "item-3-id-exit": {
			process.exit(0);
			break;
		}
	}
});

```