---
name: Bug report
about: 'Help report the weirdest of crashes in hopes of a fix '
title: ''
labels: bug
assignees: ''

---

**What happens?**
Describe what happened when you experienced this bug.

**How did it happen?**
Describe the steps taken in order to recreate the bug.
1. Open Geometry Dash
2. Go into online levels
3. Play weekly

**What was supposed to happen?**
Explain what you think should have happened.

**Log**
Please enable rich presence logging through `gdrpc.toml`.
Find the section labeled `[settings]` or add one yourself:
```
[settings]
file_version = 2
logging = true
```
make sure logging is set to `true`.  
In the game directory, you'll find a `gdrpc.log`. Send that.
