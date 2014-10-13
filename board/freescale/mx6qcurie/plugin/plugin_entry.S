
.extern plugin_start

.section .init
.globl plugin_entry

plugin_entry:
	b plugin_start

