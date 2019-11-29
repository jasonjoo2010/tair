SUBDIRS=src scripts share
DIST_SUBDIRS = src scripts share

utest: check

.PHONY: utest
EXTRA_DIST=bootstrap.sh \
	   share/configserver.conf.default share/dataserver.conf.default share/group.conf.default share/invalserver.conf.default share/proxyserver.conf.default \
		 share/lens.conf.default share/stat_collector.conf.default share/tair_administrator.conf.default share/Makefile.am share/Makefile.in \
	   scripts/Makefile.am scripts/Makefile.in scripts/mdbshm_reader.py scripts/set_shm.sh \
		 scripts/tair.sh scripts/stat.sh scripts/check_version.sh scripts/do_dump.sh scripts/do_upload.sh \
	   libtool \
	   packages/$(PACKAGE).spec

TMP_DIR:=/tmp/tair-tmp.$(shell echo $$$$)
#PREFIX:=$(shell [ x$(PREFIX) = x ] && PREFIX=/home/admin/tair && echo "$${PREFIX}")
PREFIX:=$(shell [ x$(PREFIX) = x ] && PREFIX=/usr/local/tair-$(VERSION) && echo "$${PREFIX}")
REPO_URL := $(shell git remote get-url origin)
REPO_VER := $(shell git show |head -1 |awk '{print $2}')

rpms: dist-gzip packages/$(PACKAGE).spec
	mkdir -p $(TMP_DIR)
	mkdir -p $(TMP_DIR)/BUILD
	mkdir -p $(TMP_DIR)/RPMS
	mkdir -p $(TMP_DIR)/SOURCES
	mkdir -p $(TMP_DIR)/SRPMS
	cp $(PACKAGE)-${VERSION}.tar.gz $(TMP_DIR)/SOURCES
	echo $(PREFIX)
	cd $(TMP_DIR)/BUILD; \
	tar xfz $(TMP_DIR)/SOURCES/$(PACKAGE)-${VERSION}.tar.gz \
		$(PACKAGE)-$(VERSION)/packages/$(PACKAGE).spec
	rpmbuild --define "_topdir $(TMP_DIR)" \
		 --define "NAME $(PACKAGE)" \
		 --define "VERSION $(VERSION)" \
		 --define "svn $(REPO_URL)@$(REPO_VER)" \
		 --define "_prefix $(PREFIX)" \
		-ba $(TMP_DIR)/BUILD/$(PACKAGE)-$(VERSION)/packages/$(PACKAGE).spec
	find $(TMP_DIR)/RPMS/ -name "*.rpm" -exec mv '{}' . \;
	rm -rf $(TMP_DIR)


