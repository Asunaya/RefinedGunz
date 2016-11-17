# Launcher

A basic command-line launcher application for automatic updating.

It works by downloading a patch.xml from the update server. Then, for each file, if the file is found in launcher_cache.xml, it checks if the MD5 is the same as the one in the patch.xml, and if the cached file size and last time changed match the current file's values. If so, no update is performed and no new MD5 is calculated. Otherwise, or if the file is not found in the cache, it calculates the current MD5, downloads a new file if necessary, and saves the file stats to the cache.

Caching file stats in this way lets it start the game almost instantly when there's no patch and when files are unchanged (which is the general case).

makepatch.py is the tool for generating a new patch.xml.

TODO: Add rsync-like mechanism for synchronizing old files with new files rather than downloading the whole file from scratch.