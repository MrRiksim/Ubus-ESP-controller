include $(TOPDIR)/rules.mk

PKG_NAME:=ubus-esp-controller
PKG_RELEASE:=1
PKG_VERSION:=1.0.0

include $(INCLUDE_DIR)/package.mk

define Package/ubus-esp-controller
	CATEGORY:=Base system
	TITLE:=ubus-esp-controller
	DEPENDS:=+libserialport +libblobmsg-json +libubox +libubus
endef

define Package/ubus-esp-controller/description
	A UBUS program to control an ESP microcontroller over serial
endef

define Package/ubus-esp-controller/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ubus-esp-controller $(1)/usr/bin
	$(INSTALL_BIN) ./files/ubus-esp-controller.init $(1)/etc/init.d/ubus-esp-controller
endef

$(eval $(call BuildPackage,ubus-esp-controller))