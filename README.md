# Homeplug-AV utilities

I'm writing this utilities in order to query my pair of TP-Link TL-PA211 (aka AV200).

This device is a PowerLine station. TP-Link provide only Win/macOS binary for
querying informations and change parameters, so I'm writing my own software to do so.

Right now, there are only two binaries:

* `discover`: which launch a query to discover the device
* `querynetinfo`: which query a single device (by MAC Address) in order to get its network info

Contributing/testing are welcome! :-)
