# Voyage Century Online/Bounty Bay Online fixes

### Core.exe
This is replacement for game's engine main executable. It has new message/render loop which reduces visual lag and fps issues.

Message loop before fix:
```
while(true)
{
  gotMessage = false
  while(getMessage()) {
    gotMessage = true
    processMessage()
  }
  if (gotMessage)
    Render()	
}
```

Problem is if there are many messages (like mouse movement), function `Render` will be called less often and more inconsistent.

Mesasge loop after fix:
```
while(true)
{
  if(getMessage()) {
    processMessage()
  }
  Render()
}
```

This loop can be customized in `vco_bbo_fix.ini` file

### MessageStackFix.dll
VCO's engine stores window messages in a stack with size of 256. It is easily flooded with mouse movement so there is no space for other events. This module dismisses some redundant messages or decreases its rate.

## Installation:
1. Get the latest [release](https://github.com/NarutoUA/vco_bbo_fix/releases)
- Important: take backup of `Core.exe` before this step
2. Extract archive `vco_bbo_fix.zip` to `voyage` folder (e.g. \Voyage Century Online\voyage\)
3. Check `vco_bbo_fix.ini` for more settings and details
4. You may need to reinstall everything again after game update

## Youtube demo:
[![youtube](http://img.youtube.com/vi/ynDUmCEdXrY/0.jpg)](https://www.youtube.com/watch?v=ynDUmCEdXrY "Demonstration")
