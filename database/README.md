# Legacy DOS inventory

`decomp.db`, `schema.sql`, and `dump.sql` have not been migrated to the authentic
Windows target. Their completion labels and Windows-address cross-references are
historical/unverified and are not active Windows progress. Do not export them over
new Windows evidence or synchronize them to an unverified Ghidra program.

For this bounded milestone, the active Windows inventory is
[`docs/tracking/windows_inventory.json`](../docs/tracking/windows_inventory.json);
see the [startup map](../docs/ghidra/windows_startup.md) for provenance and validation.
Replacing/resetting the broader database and dashboard pipeline is deferred.
