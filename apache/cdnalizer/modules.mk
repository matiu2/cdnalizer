mod_cdnalizer.la: mod_cdnalizer.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_cdnalizer.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_cdnalizer.la
