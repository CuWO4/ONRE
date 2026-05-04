# requires TARGET_HEADER DEBUG_TARGET_HEADER

-include define.mk

SRCDIR := $(BASE_DIR)/src
SRC_HEADERS := $(call rwildcard,$(BASE_DIR)/src,*.hxx)

define GENERATE_HEADER # arg1: target path; arg2: whether need `#line` tag
	set -eu; \
	{ \
		for hdr in $$(printf '%s\n' $(SRC_HEADERS) | sort); do \
			name=$${hdr##*/}; \
			printf '__onre_root__ %s\n' "$$name"; \
			awk ' \
				/^\/\/ === snippet begin ===$$/ {exit} \
				/^#include "[^"]+"/ { \
					dep = $$0; \
					sub(/^#include "/, "", dep); \
					sub(/"$$/, "", dep); \
					print dep " " "'"$$name"'"; \
				}' "$$hdr"; \
		done; \
	} | tsort | while IFS= read -r hdr; do \
		[ "$$hdr" = "__onre_root__" ] && continue; \
		awk ' \
			BEGIN {emit=0; started=0} \
			/^\/\/ === snippet begin ===$$/ {line=NR+1; emit=1; next} \
			/^\/\/ === snippet end ===$$/ {emit=0; next} \
			emit { \
				if ($2 && !started) { \
					print "#line " line " \"'"$(SRCDIR)/$$hdr"'\""; \
					started=1; \
				} \
				print; \
			} \
			END { if ($2) { print "#line 1 \"$1\""; } } \
		' "$(SRCDIR)/$$hdr"; \
	done | { \
		printf '%s\n%s\n\n' '#ifndef ONRE_REGEX_HPP_xC8ZmzgF_' '#define ONRE_REGEX_HPP_xC8ZmzgF_'; \
		cat; \
		printf '\n%s\n' '#endif /* #ifndef ONRE_REGEX_HPP_xC8ZmzgF_ */'; \
	} > "$1"
endef

$(TARGET_HEADER) : $(SRC_HEADERS)
	$(info BUILD   $@)
	@$(call MKDIR_P,$(dir $@))
	@$(call GENERATE_HEADER,$@,0)

$(DEBUG_TARGET_HEADER) : $(SRC_HEADERS)
	$(info BUILD   $@)
	@$(call MKDIR_P,$(dir $@))
	@$(call GENERATE_HEADER,$@,1)
