1.1 i2c总线
i2c总线用于管理i2c设备和i2c驱动，维护一个设备链表和驱动链表，定义了设备和驱动的匹配规则，定义了匹配成功后的行为，其在内核中的定义如下
struct bus_type i2c_bus_type = {
	.name		= "i2c",
	.match		= i2c_device_match,//匹配规则
	.probe		= i2c_device_probe,//匹配后的行为
	.remove		= i2c_device_remove,
	.shutdown	= i2c_device_shutdown,
};

1.2 i2c设备
i2c设备描述了i2c设备的硬件信息，例如i2c设备的地址，i2c设备在接在哪一个i2c控制器上
struct i2c_client {
	unsigned short flags;//描述i2c从设备特性的标志位		/* div., see below		*/
	unsigned short addr;//设备地址	/* chip address - NOTE: 7bit	*/
					/* addresses are stored in the	*/
					/* _LOWER_ 7 bits		*/
	char name[I2C_NAME_SIZE];//设备名称
	struct i2c_adapter *adapter;//适配器，指I2C控制器	/* the adapter we sit on	*/
	struct device dev;//表明这是一个设备		/* the device structure		*/
	int irq;//中断号			/* irq issued by device		*/
	struct list_head detected;//作为链表节点挂接到与他匹配成功的i2c-devicer 相应的链表头上
#if IS_ENABLED(CONFIG_I2C_SLAVE)// 回调函数
	i2c_slave_cb_t slave_cb;	/* callback for slave mode	*/
#endif
};


/**
 * struct i2c_algorithm - represent I2C transfer method
 * @master_xfer: Issue a set of i2c transactions to the given I2C adapter
 *   defined by the msgs array, with num messages available to transfer via
 *   the adapter specified by adap.
 * @smbus_xfer: Issue smbus transactions to the given I2C adapter. If this
 *   is not present, then the bus layer will try and convert the SMBus calls
 *   into I2C transfers instead.
 * @functionality: Return the flags that this algorithm/adapter pair supports
 *   from the I2C_FUNC_* flags.
 * @reg_slave: Register given client to I2C slave mode of this adapter
 * @unreg_slave: Unregister given client from I2C slave mode of this adapter
 *
 * The following structs are for those who like to implement new bus drivers:
 * i2c_algorithm is the interface to a class of hardware solutions which can
 * be addressed using the same bus algorithms - i.e. bit-banging or the PCF8584
 * to name two of the most common.
 *
 * The return codes from the @master_xfer field should indicate the type of
 * error code that occurred during the transfer, as documented in the kernel
 * Documentation file Documentation/i2c/fault-codes.
 */
struct i2c_algorithm {
	/* If an adapter algorithm can't do I2C-level access, set master_xfer
	   to NULL. If an adapter algorithm can do SMBus access, set
	   smbus_xfer. If set to NULL, the SMBus protocol is simulated
	   using common I2C messages */
	/* master_xfer should return the number of messages successfully
	   processed, or a negative value on error */
	int (*master_xfer)(struct i2c_adapter *adap, struct i2c_msg *msgs,
			   int num);
	int (*smbus_xfer) (struct i2c_adapter *adap, u16 addr,
			   unsigned short flags, char read_write,
			   u8 command, int size, union i2c_smbus_data *data);

	/* To determine what the adapter supports */
	u32 (*functionality) (struct i2c_adapter *);

#if IS_ENABLED(CONFIG_I2C_SLAVE)
	int (*reg_slave)(struct i2c_client *client);
	int (*unreg_slave)(struct i2c_client *client);
#endif
};

/**
 * struct device - The basic device structure
 * @parent:	The device's "parent" device, the device to which it is attached.
 * 		In most cases, a parent device is some sort of bus or host
 * 		controller. If parent is NULL, the device, is a top-level device,
 * 		which is not usually what you want.
 * @p:		Holds the private data of the driver core portions of the device.
 * 		See the comment of the struct device_private for detail.
 * @kobj:	A top-level, abstract class from which other classes are derived.
 * @init_name:	Initial name of the device.
 * @type:	The type of device.
 * 		This identifies the device type and carries type-specific
 * 		information.
 * @mutex:	Mutex to synchronize calls to its driver.
 * @bus:	Type of bus device is on.
 * @driver:	Which driver has allocated this
 * @platform_data: Platform data specific to the device.
 * 		Example: For devices on custom boards, as typical of embedded
 * 		and SOC based hardware, Linux often uses platform_data to point
 * 		to board-specific structures describing devices and how they
 * 		are wired.  That can include what ports are available, chip
 * 		variants, which GPIO pins act in what additional roles, and so
 * 		on.  This shrinks the "Board Support Packages" (BSPs) and
 * 		minimizes board-specific #ifdefs in drivers.
 * @driver_data: Private pointer for driver specific info.
 * @power:	For device power management.
 * 		See Documentation/power/devices.txt for details.
 * @pm_domain:	Provide callbacks that are executed during system suspend,
 * 		hibernation, system resume and during runtime PM transitions
 * 		along with subsystem-level and driver-level callbacks.
 * @pins:	For device pin management.
 *		See Documentation/pinctrl.txt for details.
 * @msi_list:	Hosts MSI descriptors
 * @msi_domain: The generic MSI domain this device is using.
 * @numa_node:	NUMA node this device is close to.
 * @dma_mask:	Dma mask (if dma'ble device).
 * @coherent_dma_mask: Like dma_mask, but for alloc_coherent mapping as not all
 * 		hardware supports 64-bit addresses for consistent allocations
 * 		such descriptors.
 * @dma_pfn_offset: offset of DMA memory range relatively of RAM
 * @dma_parms:	A low level driver may set these to teach IOMMU code about
 * 		segment limitations.
 * @dma_pools:	Dma pools (if dma'ble device).
 * @dma_mem:	Internal for coherent mem override.
 * @cma_area:	Contiguous memory area for dma allocations
 * @archdata:	For arch-specific additions.
 * @of_node:	Associated device tree node.
 * @fwnode:	Associated device node supplied by platform firmware.
 * @devt:	For creating the sysfs "dev".
 * @id:		device instance
 * @devres_lock: Spinlock to protect the resource of the device.
 * @devres_head: The resources list of the device.
 * @knode_class: The node used to add the device to the class list.
 * @class:	The class of the device.
 * @groups:	Optional attribute groups.
 * @release:	Callback to free the device after all references have
 * 		gone away. This should be set by the allocator of the
 * 		device (i.e. the bus driver that discovered the device).
 * @iommu_group: IOMMU group the device belongs to.
 *
 * @offline_disabled: If set, the device is permanently online.
 * @offline:	Set after successful invocation of bus type's .offline().
 *
 * At the lowest level, every device in a Linux system is represented by an
 * instance of struct device. The device structure contains the information
 * that the device model core needs to model the system. Most subsystems,
 * however, track additional information about the devices they host. As a
 * result, it is rare for devices to be represented by bare device structures;
 * instead, that structure, like kobject structures, is usually embedded within
 * a higher-level representation of the device.
 */
struct device {
	struct device		*parent;

	struct device_private	*p;

	struct kobject kobj;
	const char		*init_name; /* initial name of the device */
	const struct device_type *type;

	struct mutex		mutex;	/* mutex to synchronize calls to
					 * its driver.
					 */

	struct bus_type	*bus;		/* type of bus device is on */
	struct device_driver *driver;	/* which driver has allocated this
					   device */
	void		*platform_data;	/* Platform specific data, device
					   core doesn't touch it */
	void		*driver_data;	/* Driver data, set and get with
					   dev_set/get_drvdata */
	struct dev_pm_info	power;
	struct dev_pm_domain	*pm_domain;

#ifdef CONFIG_GENERIC_MSI_IRQ_DOMAIN
	struct irq_domain	*msi_domain;
#endif
#ifdef CONFIG_PINCTRL
	struct dev_pin_info	*pins;
#endif
#ifdef CONFIG_GENERIC_MSI_IRQ
	struct list_head	msi_list;
#endif

#ifdef CONFIG_NUMA
	int		numa_node;	/* NUMA node this device is close to */
#endif
	u64		*dma_mask;	/* dma mask (if dma'able device) */
	u64		coherent_dma_mask;/* Like dma_mask, but for
					     alloc_coherent mappings as
					     not all hardware supports
					     64 bit addresses for consistent
					     allocations such descriptors. */
	unsigned long	dma_pfn_offset;

	struct device_dma_parameters *dma_parms;

	struct list_head	dma_pools;	/* dma pools (if dma'ble) */

	struct dma_coherent_mem	*dma_mem; /* internal for coherent mem
					     override */
#ifdef CONFIG_DMA_CMA
	struct cma *cma_area;		/* contiguous memory area for dma
					   allocations */
#endif
	/* arch specific additions */
	struct dev_archdata	archdata;

	struct device_node	*of_node; /* associated device tree node */
	struct fwnode_handle	*fwnode; /* firmware device node */

	dev_t			devt;	/* dev_t, creates the sysfs "dev" */
	u32			id;	/* device instance */

	spinlock_t		devres_lock;
	struct list_head	devres_head;

	struct klist_node	knode_class;
	struct class		*class;
	const struct attribute_group **groups;	/* optional groups */

	void	(*release)(struct device *dev);
	struct iommu_group	*iommu_group;

	bool			offline_disabled:1;
	bool			offline:1;
};

1.3 i2c驱动
i2c驱动是i2c设备的驱动程序，用于匹配i2c设备
struct i2c_driver {
	unsigned int class;// 我们实例化什么样的i2c设备(用于检测)

	/* Notifies the driver that a new bus has appeared. You should avoid
	 * using this, it will be removed in a near future.
	 */
	int (*attach_adapter)(struct i2c_adapter *) __deprecated;

	/* Standard driver model interfaces */
	int (*probe)(struct i2c_client *, const struct i2c_device_id *);
	int (*remove)(struct i2c_client *);

	/* driver model interfaces that don't relate to enumeration  */
	void (*shutdown)(struct i2c_client *);

	/* Alert callback, for example for the SMBus alert protocol.
	 * The format and meaning of the data value depends on the protocol.
	 * For the SMBus alert protocol, there is a single bit of data passed
	 * as the alert response's low bit ("event flag").
	 */
	void (*alert)(struct i2c_client *, unsigned int data);

	/* a ioctl like command that can be used to perform specific functions
	 * with the device.
	 */
	int (*command)(struct i2c_client *client, unsigned int cmd, void *arg);

	struct device_driver driver;// 表明这是一个驱动
	const struct i2c_device_id *id_table;// 要匹配的从设备信息

	/* Device detection callback for automatic device creation */
	int (*detect)(struct i2c_client *, struct i2c_board_info *);// 设备探测函数
	const unsigned short *address_list;// 设备地址
	struct list_head clients;//当前设备链表 we created
};

/**
 * struct device_driver - The basic device driver structure
 * @name:	Name of the device driver.
 * @bus:	The bus which the device of this driver belongs to.
 * @owner:	The module owner.
 * @mod_name:	Used for built-in modules.
 * @suppress_bind_attrs: Disables bind/unbind via sysfs.
 * @probe_type:	Type of the probe (synchronous or asynchronous) to use.
 * @of_match_table: The open firmware table.
 * @acpi_match_table: The ACPI match table.
 * @probe:	Called to query the existence of a specific device,
 *		whether this driver can work with it, and bind the driver
 *		to a specific device.
 * @remove:	Called when the device is removed from the system to
 *		unbind a device from this driver.
 * @shutdown:	Called at shut-down time to quiesce the device.
 * @suspend:	Called to put the device to sleep mode. Usually to a
 *		low power state.
 * @resume:	Called to bring a device from sleep mode.
 * @groups:	Default attributes that get created by the driver core
 *		automatically.
 * @pm:		Power management operations of the device which matched
 *		this driver.
 * @p:		Driver core's private data, no one other than the driver
 *		core can touch this.
 *
 * The device driver-model tracks all of the drivers known to the system.
 * The main reason for this tracking is to enable the driver core to match
 * up drivers with new devices. Once drivers are known objects within the
 * system, however, a number of other things become possible. Device drivers
 * can export information and configuration variables that are independent
 * of any specific device.
 */


struct device_driver {
	const char		*name;
	struct bus_type		*bus;

	struct module		*owner;
	const char		*mod_name;	/* used for built-in modules */

	bool suppress_bind_attrs;	/* disables bind/unbind via sysfs */
	enum probe_type probe_type;

	const struct of_device_id	*of_match_table;
	const struct acpi_device_id	*acpi_match_table;

	int (*probe) (struct device *dev);
	int (*remove) (struct device *dev);
	void (*shutdown) (struct device *dev);
	int (*suspend) (struct device *dev, pm_message_t state);
	int (*resume) (struct device *dev);
	const struct attribute_group **groups;

	const struct dev_pm_ops *pm;

	struct driver_private *p;
};

typedef unsigned long kernel_ulong_t;

struct i2c_device_id {
	char name[I2C_NAME_SIZE];
	kernel_ulong_t driver_data;	/* Data private to the driver */
};

1.4 i2c适配器
i2c适配器是soc上的i2c控制器的软件抽象，可以通过其定义的算法想硬件设备传输数据，其结构体定义如下

/*
 * i2c_adapter is the structure used to identify a physical i2c bus along
 * with the access algorithms necessary to access it.
 */
//这个结构描述一个i2c适配器，指的就是soc上的具体i2c控制器，控制器驱动需要实现i2c读写时需要用到的核心算法回调
struct i2c_adapter {
	struct module *owner;
	unsigned int class;		  /* classes to allow probing for */
	const struct i2c_algorithm *algo; // 算法 /* the algorithm to access the bus */
	void *algo_data;

	/* data fields that are valid for all devices	*/
	struct rt_mutex bus_lock;

	int timeout;			/* in jiffies */
	int retries;
	struct device dev;		/* the adapter device */

	int nr;
	char name[48];
	struct completion dev_released;

	struct mutex userspace_clients_lock;
	struct list_head userspace_clients;

	struct i2c_bus_recovery_info *bus_recovery_info;
	const struct i2c_adapter_quirks *quirks;
};

其中的i2c_algorithm表示算法，用于向硬件设备传输数据，其定义如下

/**
 * struct i2c_algorithm - represent I2C transfer method
 * @master_xfer: Issue a set of i2c transactions to the given I2C adapter
 *   defined by the msgs array, with num messages available to transfer via
 *   the adapter specified by adap.
 * @smbus_xfer: Issue smbus transactions to the given I2C adapter. If this
 *   is not present, then the bus layer will try and convert the SMBus calls
 *   into I2C transfers instead.
 * @functionality: Return the flags that this algorithm/adapter pair supports
 *   from the I2C_FUNC_* flags.
 * @reg_slave: Register given client to I2C slave mode of this adapter
 * @unreg_slave: Unregister given client from I2C slave mode of this adapter
 *
 * The following structs are for those who like to implement new bus drivers:
 * i2c_algorithm is the interface to a class of hardware solutions which can
 * be addressed using the same bus algorithms - i.e. bit-banging or the PCF8584
 * to name two of the most common.
 *
 * The return codes from the @master_xfer field should indicate the type of
 * error code that occurred during the transfer, as documented in the kernel
 * Documentation file Documentation/i2c/fault-codes.
 */
struct i2c_algorithm {
	/* If an adapter algorithm can't do I2C-level access, set master_xfer
	   to NULL. If an adapter algorithm can do SMBus access, set
	   smbus_xfer. If set to NULL, the SMBus protocol is simulated
	   using common I2C messages */
	/* master_xfer should return the number of messages successfully
	   processed, or a negative value on error */
	int (*master_xfer)(struct i2c_adapter *adap, struct i2c_msg *msgs,
			   int num);
	int (*smbus_xfer) (struct i2c_adapter *adap, u16 addr,
			   unsigned short flags, char read_write,
			   u8 command, int size, union i2c_smbus_data *data);

	/* To determine what the adapter supports */
	u32 (*functionality) (struct i2c_adapter *);

#if IS_ENABLED(CONFIG_I2C_SLAVE)
	int (*reg_slave)(struct i2c_client *client);
	int (*unreg_slave)(struct i2c_client *client);
#endif
};

总结一下：i2c驱动的主要对象是i2c总线，i2c设备，i2c驱动，i2c适配器

i2c总线用于管理i2c设备和i2c驱动

i2c设备描述了i2c设备的硬件信息

i2c驱动是i2c设备对应的驱动程序

i2c是适配器是soc上的i2c控制器，其定义了算法，可以向i2c硬件设备传输数据

其中直接面向编写i2c设备驱动的开发者是i2c设备和i2c驱动，i2c总线和i2c适配器是幕后工作者

二、linux i2c驱动框架

i2c驱动框架可以分为四部分，i2c核心，i2c设备，i2c驱动，i2c适配器，其中i2c总线位于i2c核心中可以用下图来总结

i2c核心维护着一条i2c总线，提供了注册i2c设备，i2c驱动，i2c适配器的接口

i2c总线维护着一条设备链表和驱动链表，当向i2c核心层注册设备时，会将其添加到总线的设备链表中，然后遍历总线上的驱动链表，查看二者是否匹配，如果匹配就调用驱动的probe函数

在i2c驱动程序中，通过i2c适配器中的算法想i2c硬件设备传输数据

三、i2c驱动框架源码剖析

3.1 注册i2c设备

注册i2c设备可以通过struct i2c_client *i2c_new_device(struct i2c_adapter *adap, struct i2c_board_info const *info)，
此函数会生成一个i2c_clinet，指定对应的总线为i2c总线，然后向总线注册设备
/**
 * i2c_new_device - instantiate an i2c device
 * @adap: the adapter managing the device
 * @info: describes one I2C device; bus_num is ignored
 * Context: can sleep
 *
 * Create an i2c device. Binding is handled through driver model
 * probe()/remove() methods.  A driver may be bound to this device when we
 * return from this function, or any later moment (e.g. maybe hotplugging will
 * load the driver module).  This call is not appropriate for use by mainboard
 * initialization logic, which usually runs during an arch_initcall() long
 * before any i2c_adapter could exist.
 *
 * This returns the new i2c client, which may be saved for later use with
 * i2c_unregister_device(); or NULL to indicate an error.
 */
struct i2c_client *
i2c_new_device(struct i2c_adapter *adap, struct i2c_board_info const *info)
{
	struct i2c_client	*client;
	int			status;

	client = kzalloc(sizeof *client, GFP_KERNEL);
	if (!client)
		return NULL;
/* 关联client设备挂载的adapter设备 */
	client->adapter = adap;

	client->dev.platform_data = info->platform_data;

	if (info->archdata)
		client->dev.archdata = *info->archdata;

	client->flags = info->flags;
	/* 设置地址 */
	client->addr = info->addr;
	client->irq = info->irq;

	strlcpy(client->name, info->type, sizeof(client->name));

	status = i2c_check_addr_validity(client->addr, client->flags);
	if (status) {
		dev_err(&adap->dev, "Invalid %d-bit I2C address 0x%02hx\n",
			client->flags & I2C_CLIENT_TEN ? 10 : 7, client->addr);
		goto out_err_silent;
	}

	/* Check for address business */
	status = i2c_check_addr_ex(adap, i2c_encode_flags_to_addr(client));
	if (status != 0)
		dev_err(&adap->dev, "%d i2c clients have been registered at 0x%02x",
			status, client->addr);
	client->dev.parent = &client->adapter->dev;
	client->dev.bus = &i2c_bus_type;
	client->dev.type = &i2c_client_type;
	client->dev.of_node = info->of_node;
	client->dev.fwnode = info->fwnode;

	i2c_dev_set_name(adap, client, status);
	/* 正式注册到i2c总线 */
	status = device_register(&client->dev);
	if (status)
		goto out_err;

	dev_dbg(&adap->dev, "client [%s] registered with bus id %s\n",
		client->name, dev_name(&client->dev));

	return client;

out_err:
	dev_err(&adap->dev, "Failed to register i2c client %s at 0x%02x "
		"(%d)\n", client->name, client->addr, status);
out_err_silent:
	kfree(client);
	return NULL;
}
EXPORT_SYMBOL_GPL(i2c_new_device);

看一下其中的i2c_bus_type对象，其表示i2c总线，定义了设备和驱动的匹配规则还有匹配成功后的行为
struct bus_type i2c_bus_type = {
	.name		= "i2c",
	.match		= i2c_device_match,//匹配规则
	.probe		= i2c_device_probe,//匹配成功后的行为
	.remove		= i2c_device_remove,
	.shutdown	= i2c_device_shutdown,
};
EXPORT_SYMBOL_GPL(i2c_bus_type);

下面再来看看device_register向总线注册设备过中会发生什么

device_register首先会将设备添加到总线的设备链表中，然后遍历总线的驱动链表，判断设备和驱动是否匹配，如果匹配就去掉驱动的probe函数，下面看一看源码分析
/**
 * device_register - register a device with the system.
 * @dev: pointer to the device structure
 *
 * This happens in two clean steps - initialize the device
 * and add it to the system. The two steps can be called
 * separately, but this is the easiest and most common.
 * I.e. you should only call the two helpers separately if
 * have a clearly defined need to use and refcount the device
 * before it is added to the hierarchy.
 *
 * For more information, see the kerneldoc for device_initialize()
 * and device_add().
 *
 * NOTE: _Never_ directly free @dev after calling this function, even
 * if it returned an error! Always use put_device() to give up the
 * reference initialized in this function instead.
 */
int device_register(struct device *dev)
{
	device_initialize(dev);
	return device_add(dev);
}
EXPORT_SYMBOL_GPL(device_register);

/**
 * device_add - add device to device hierarchy.
 * @dev: device.
 *
 * This is part 2 of device_register(), though may be called
 * separately _iff_ device_initialize() has been called separately.
 *
 * This adds @dev to the kobject hierarchy via kobject_add(), adds it
 * to the global and sibling lists for the device, then
 * adds it to the other relevant subsystems of the driver model.
 *
 * Do not call this routine or device_register() more than once for
 * any device structure.  The driver model core is not designed to work
 * with devices that get unregistered and then spring back to life.
 * (Among other things, it's very hard to guarantee that all references
 * to the previous incarnation of @dev have been dropped.)  Allocate
 * and register a fresh new struct device instead.
 *
 * NOTE: _Never_ directly free @dev after calling this function, even
 * if it returned an error! Always use put_device() to give up your
 * reference instead.
 */
int device_add(struct device *dev)
{
	struct device *parent = NULL;
	struct kobject *kobj;
	struct class_interface *class_intf;
	int error = -EINVAL;
	struct kobject *glue_dir = NULL;

	dev = get_device(dev);
	if (!dev)
		goto done;

	if (!dev->p) {
		error = device_private_init(dev);
		if (error)
			goto done;
	}

	/*
	 * for statically allocated devices, which should all be converted
	 * some day, we need to initialize the name. We prevent reading back
	 * the name, and force the use of dev_name()
	 */
	if (dev->init_name) {
		dev_set_name(dev, "%s", dev->init_name);
		dev->init_name = NULL;
	}

	/* subsystems can specify simple device enumeration */
	if (!dev_name(dev) && dev->bus && dev->bus->dev_name)
		dev_set_name(dev, "%s%u", dev->bus->dev_name, dev->id);

	if (!dev_name(dev)) {
		error = -EINVAL;
		goto name_error;
	}

	pr_debug("device: '%s': %s\n", dev_name(dev), __func__);

	parent = get_device(dev->parent);
	kobj = get_device_parent(dev, parent);
	if (IS_ERR(kobj)) {
		error = PTR_ERR(kobj);
		goto parent_error;
	}
	if (kobj)
		dev->kobj.parent = kobj;

	/* use parent numa_node */
	if (parent && (dev_to_node(dev) == NUMA_NO_NODE))
		set_dev_node(dev, dev_to_node(parent));

	/* first, register with generic layer. */
	/* we require the name to be set before, and pass NULL */
	error = kobject_add(&dev->kobj, dev->kobj.parent, NULL);
	if (error) {
		glue_dir = get_glue_dir(dev);
		goto Error;
	}

	/* notify platform of device entry */
	if (platform_notify)
		platform_notify(dev);

	error = device_create_file(dev, &dev_attr_uevent);
	if (error)
		goto attrError;

	error = device_add_class_symlinks(dev);
	if (error)
		goto SymlinkError;
	error = device_add_attrs(dev);
	if (error)
		goto AttrsError;
	error = bus_add_device(dev);
	if (error)
		goto BusError;
	error = dpm_sysfs_add(dev);
	if (error)
		goto DPMError;
	device_pm_add(dev);

	if (MAJOR(dev->devt)) {
		error = device_create_file(dev, &dev_attr_dev);
		if (error)
			goto DevAttrError;

		error = device_create_sys_dev_entry(dev);
		if (error)
			goto SysEntryError;

		devtmpfs_create_node(dev);
	}

	/* Notify clients of device addition.  This call must come
	 * after dpm_sysfs_add() and before kobject_uevent().
	 */
	if (dev->bus)
		blocking_notifier_call_chain(&dev->bus->p->bus_notifier,
					     BUS_NOTIFY_ADD_DEVICE, dev);

	kobject_uevent(&dev->kobj, KOBJ_ADD);
	bus_probe_device(dev);
	if (parent)
		klist_add_tail(&dev->p->knode_parent,
			       &parent->p->klist_children);

	if (dev->class) {
		mutex_lock(&dev->class->p->mutex);
		/* tie the class to the device */
		klist_add_tail(&dev->knode_class,
			       &dev->class->p->klist_devices);

		/* notify any interfaces that the device is here */
		list_for_each_entry(class_intf,
				    &dev->class->p->interfaces, node)
			if (class_intf->add_dev)
				class_intf->add_dev(dev, class_intf);
		mutex_unlock(&dev->class->p->mutex);
	}
done:
	put_device(dev);
	return error;
 SysEntryError:
	if (MAJOR(dev->devt))
		device_remove_file(dev, &dev_attr_dev);
 DevAttrError:
	device_pm_remove(dev);
	dpm_sysfs_remove(dev);
 DPMError:
	bus_remove_device(dev);
 BusError:
	device_remove_attrs(dev);
 AttrsError:
	device_remove_class_symlinks(dev);
 SymlinkError:
	device_remove_file(dev, &dev_attr_uevent);
 attrError:
	kobject_uevent(&dev->kobj, KOBJ_REMOVE);
	glue_dir = get_glue_dir(dev);
	kobject_del(&dev->kobj);
 Error:
	cleanup_glue_dir(dev, glue_dir);
parent_error:
	put_device(parent);
name_error:
	kfree(dev->p);
	dev->p = NULL;
	goto done;
}
EXPORT_SYMBOL_GPL(device_add);

其中bus_add_device函数会将设备添加到总线设备链表中，如下

/**
 * bus_add_device - add device to bus
 * @dev: device being added
 *
 * - Add device's bus attributes.
 * - Create links to device's bus.
 * - Add the device to its bus's list of devices.
 */
int bus_add_device(struct device *dev)
{
	struct bus_type *bus = bus_get(dev->bus);
	int error = 0;

	if (bus) {
		pr_debug("bus: '%s': add device %s\n", bus->name, dev_name(dev));
		error = device_add_attrs(bus, dev);
		if (error)
			goto out_put;
		error = device_add_groups(dev, bus->dev_groups);
		if (error)
			goto out_id;
		error = sysfs_create_link(&bus->p->devices_kset->kobj,
						&dev->kobj, dev_name(dev));
		if (error)
			goto out_groups;
		error = sysfs_create_link(&dev->kobj,
				&dev->bus->p->subsys.kobj, "subsystem");
		if (error)
			goto out_subsys;
		klist_add_tail(&dev->p->knode_bus, &bus->p->klist_devices);
	}
	return 0;

out_subsys:
	sysfs_remove_link(&bus->p->devices_kset->kobj, dev_name(dev));
out_groups:
	device_remove_groups(dev, bus->dev_groups);
out_id:
	device_remove_attrs(bus, dev);
out_put:
	bus_put(dev->bus);
	return error;
}

bus_probe_device函数会遍历总线的驱动链表，如下
/**
 * bus_probe_device - probe drivers for a new device
 * @dev: device to probe
 *
 * - Automatically probe for a driver if the bus allows it.
 */
void bus_probe_device(struct device *dev)
{
	struct bus_type *bus = dev->bus;
	struct subsys_interface *sif;

	if (!bus)
		return;

	if (bus->p->drivers_autoprobe)
		device_initial_probe(dev);

	mutex_lock(&bus->p->mutex);
	list_for_each_entry(sif, &bus->p->interfaces, node)
		if (sif->add_dev)
			sif->add_dev(dev, sif);
	mutex_unlock(&bus->p->mutex);
}

/**
 * device_attach - try to attach device to a driver.
 * @dev: device.
 *
 * Walk the list of drivers that the bus has and call
 * driver_probe_device() for each pair. If a compatible
 * pair is found, break out and return.
 *
 * Returns 1 if the device was bound to a driver;
 * 0 if no matching driver was found;
 * -ENODEV if the device is not registered.
 *
 * When called for a USB interface, @dev->parent lock must be held.
 */
int device_attach(struct device *dev)
{
	return __device_attach(dev, false);
}
EXPORT_SYMBOL_GPL(device_attach);

bus_for_each_drv(dev->bus, NULL, dev, _device_attach);会遍历总线的驱动链表的每一项，然后调用_device_attach
static int __device_attach(struct device *dev, bool allow_async)
{
	int ret = 0;

	device_lock(dev);
	if (dev->driver) {
		if (klist_node_attached(&dev->p->knode_driver)) {
			ret = 1;
			goto out_unlock;
		}
		ret = device_bind_driver(dev);
		if (ret == 0)
			ret = 1;
		else {
			dev->driver = NULL;
			ret = 0;
		}
	} else {
		struct device_attach_data data = {
			.dev = dev,
			.check_async = allow_async,
			.want_async = false,
		};

		if (dev->parent)
			pm_runtime_get_sync(dev->parent);

		ret = bus_for_each_drv(dev->bus, NULL, &data,
					__device_attach_driver);
		if (!ret && allow_async && data.have_async) {
			/*
			 * If we could not find appropriate driver
			 * synchronously and we are allowed to do
			 * async probes and there are drivers that
			 * want to probe asynchronously, we'll
			 * try them.
			 */
			dev_dbg(dev, "scheduling asynchronous probe\n");
			get_device(dev);
			async_schedule(__device_attach_async_helper, dev);
		} else {
			pm_request_idle(dev);
		}

		if (dev->parent)
			pm_runtime_put(dev->parent);
	}
out_unlock:
	device_unlock(dev);
	return ret;
}

driver_match_device函数会判断设备和驱动是否匹配，如果调用driver_probe_device

首先来看看driver_match_device函数的定义

static inline int driver_match_device(struct device_driver *drv,
				      struct device *dev)
{
	return drv->bus->match ? drv->bus->match(dev, drv) : 1;
}

发现它调用了总线的match函数，这里的总线在注册i2c设备的时候已经被设置为i2c总线了，定义如下

struct bus_type i2c_bus_type = {
	.name		= "i2c",
	.match		= i2c_device_match,//匹配规则
	.probe		= i2c_device_probe,//匹配后的行为
	.remove		= i2c_device_remove,
	.shutdown	= i2c_device_shutdown,
};
EXPORT_SYMBOL_GPL(i2c_bus_type);

所以这里会调用到i2c_device_match函数，i2c_device_match会通过i2c驱动的id_table中每一个的name和i2c设备的name进行匹配

static const struct i2c_device_id *i2c_match_id(const struct i2c_device_id *id,
						const struct i2c_client *client)
{
	while (id->name[0]) {
		if (strcmp(client->name, id->name) == 0)
			return id;
		id++;
	}
	return NULL;
}


// 这个回调主要用于匹配i2c总线上的设备和驱动
static int i2c_device_match(struct device *dev, struct device_driver *drv)
{
	struct i2c_client	*client = i2c_verify_client(dev);
	struct i2c_driver	*driver;

	if (!client)
		return 0;

	/* Attempt an OF style match */
	if (of_driver_match_device(dev, drv))
		return 1;

	/* Then ACPI style match */
	if (acpi_driver_match_device(dev, drv))
		return 1;

	driver = to_i2c_driver(drv);
	/* match on an id table if there is one */
	/* 使用dts具体i2c设备节点的compatible属性后半部和i2c_driver的id_table对应字段做对比，一样就返回真。
	如rk808的compatible为"rockchip,rk808"，而rk808驱动的id_table包含"rk808" */
	if (driver->id_table)
		return i2c_match_id(driver->id_table, client) != NULL;

	return 0;
}

如果匹配成功会调用driver_probe_device，下面再来看看driver_probe_device
driver_probe_device函数最终会先调用到i2c总线的probe函数，然后再调用i2c驱动的probe函数

/**
 * driver_probe_device - attempt to bind device & driver together
 * @drv: driver to bind a device to
 * @dev: device to try to bind to the driver
 *
 * This function returns -ENODEV if the device is not registered,
 * 1 if the device is bound successfully and 0 otherwise.
 *
 * This function must be called with @dev lock held.  When called for a
 * USB interface, @dev->parent lock must be held as well.
 *
 * If the device has a parent, runtime-resume the parent before driver probing.
 */
int driver_probe_device(struct device_driver *drv, struct device *dev)
{
	int ret = 0;

	if (!device_is_registered(dev))
		return -ENODEV;

	pr_debug("bus: '%s': %s: matched device %s with driver %s\n",
		 drv->bus->name, __func__, dev_name(dev), drv->name);

	if (dev->parent)
		pm_runtime_get_sync(dev->parent);

	pm_runtime_barrier(dev);
	ret = really_probe(dev, drv);
	pm_request_idle(dev);

	if (dev->parent)
		pm_runtime_put(dev->parent);

	return ret;
}

static int really_probe(struct device *dev, struct device_driver *drv)
{
	int ret = 0;
	int local_trigger_count = atomic_read(&deferred_trigger_count);

	atomic_inc(&probe_count);
	pr_debug("bus: '%s': %s: probing driver %s with device %s\n",
		 drv->bus->name, __func__, drv->name, dev_name(dev));
	WARN_ON(!list_empty(&dev->devres_head));

	dev->driver = drv;

	/* If using pinctrl, bind pins now before probing */
	ret = pinctrl_bind_pins(dev);
	if (ret)
		goto probe_failed;

	if (driver_sysfs_add(dev)) {
		printk(KERN_ERR "%s: driver_sysfs_add(%s) failed\n",
			__func__, dev_name(dev));
		goto probe_failed;
	}

	if (dev->pm_domain && dev->pm_domain->activate) {
		ret = dev->pm_domain->activate(dev);
		if (ret)
			goto probe_failed;
	}

	if (dev->bus->probe) {
		ret = dev->bus->probe(dev);
		if (ret)
			goto probe_failed;
	} else if (drv->probe) {
		ret = drv->probe(dev);
		if (ret)
			goto probe_failed;
	}

	pinctrl_init_done(dev);

	if (dev->pm_domain && dev->pm_domain->sync)
		dev->pm_domain->sync(dev);

	driver_bound(dev);
	ret = 1;
	pr_debug("bus: '%s': %s: bound device %s to driver %s\n",
		 drv->bus->name, __func__, dev_name(dev), drv->name);
	goto done;

probe_failed:
	devres_release_all(dev);
	driver_sysfs_remove(dev);
	dev->driver = NULL;
	dev_set_drvdata(dev, NULL);
	if (dev->pm_domain && dev->pm_domain->dismiss)
		dev->pm_domain->dismiss(dev);

	switch (ret) {
	case -EPROBE_DEFER:
		/* Driver requested deferred probing */
		dev_dbg(dev, "Driver %s requests probe deferral\n", drv->name);
		driver_deferred_probe_add(dev);
		/* Did a trigger occur while probing? Need to re-trigger if yes */
		if (local_trigger_count != atomic_read(&deferred_trigger_count))
			driver_deferred_probe_trigger();
		break;
	case -ENODEV:
	case -ENXIO:
		pr_debug("%s: probe of %s rejects match %d\n",
			 drv->name, dev_name(dev), ret);
		break;
	default:
		/* driver matched but the probe failed */
		printk(KERN_WARNING
		       "%s: probe of %s failed with error %d\n",
		       drv->name, dev_name(dev), ret);
	}
	/*
	 * Ignore errors returned by ->probe so that the next driver can try
	 * its luck.
	 */
	ret = 0;
done:
	atomic_dec(&probe_count);
	wake_up(&probe_waitqueue);
	return ret;
}

总线的probe函数为i2c_device_probe，此函数会调用驱动的probe函数

// 这个回调主要在驱动和设备匹配成功后，调用驱动的probe回调执行一些初始化操作
static int i2c_device_probe(struct device *dev)
{
	struct i2c_client	*client = i2c_verify_client(dev);
	struct i2c_driver	*driver;
	int status;

	if (!client)
		return 0;

	if (!client->irq) {
		int irq = -ENOENT;

		if (dev->of_node) {
			irq = of_irq_get_byname(dev->of_node, "irq");
			if (irq == -EINVAL || irq == -ENODATA)
				irq = of_irq_get(dev->of_node, 0);
		} else if (ACPI_COMPANION(dev)) {
			irq = acpi_dev_gpio_irq_get(ACPI_COMPANION(dev), 0);
		}
		if (irq == -EPROBE_DEFER)
			return irq;
		if (irq < 0)
			irq = 0;

		client->irq = irq;
	}

	driver = to_i2c_driver(dev->driver);
	if (!driver->probe || !driver->id_table)
		return -ENODEV;

	if (client->flags & I2C_CLIENT_WAKE) {
		int wakeirq = -ENOENT;

		if (dev->of_node) {
			wakeirq = of_irq_get_byname(dev->of_node, "wakeup");
			if (wakeirq == -EPROBE_DEFER)
				return wakeirq;
		}

		device_init_wakeup(&client->dev, true);

		if (wakeirq > 0 && wakeirq != client->irq)
			status = dev_pm_set_dedicated_wake_irq(dev, wakeirq);
		else if (client->irq > 0)
			status = dev_pm_set_wake_irq(dev, client->irq);
		else
			status = 0;

		if (status)
			dev_warn(&client->dev, "failed to set up wakeup irq");
	}

	dev_dbg(dev, "probe\n");

	status = of_clk_set_defaults(dev->of_node, false);
	if (status < 0)
		goto err_clear_wakeup_irq;

	status = dev_pm_domain_attach(&client->dev, true);
	if (status == -EPROBE_DEFER)
		goto err_clear_wakeup_irq;

	status = driver->probe(client, i2c_match_id(driver->id_table, client));
	if (status)
		goto err_detach_pm_domain;

	return 0;

err_detach_pm_domain:
	dev_pm_domain_detach(&client->dev, true);
err_clear_wakeup_irq:
	dev_pm_clear_wake_irq(&client->dev);
	device_init_wakeup(&client->dev, false);
	return status;
}

3.2 注册i2c驱动

可以通过i2c_add_driver注册i2c驱动，该函数会指定驱动对应的总线为i2c总线，然后向总线注册驱动

/*
 * An i2c_driver is used with one or more i2c_client (device) nodes to access
 * i2c slave chips, on a bus instance associated with some i2c_adapter.
 */

int i2c_register_driver(struct module *owner, struct i2c_driver *driver)
{
	int res;

	/* Can't register until after driver model init */
	if (unlikely(WARN_ON(!i2c_bus_type.p)))
		return -EAGAIN;

	/* add the driver to the list of i2c drivers in the driver core */
	driver->driver.owner = owner;
	driver->driver.bus = &i2c_bus_type;// 指定i2c总线
	INIT_LIST_HEAD(&driver->clients);

	/* When registration returns, the driver core
	 * will have called probe() for all matching-but-unbound devices.
	 */
	res = driver_register(&driver->driver);// 向总线注册驱动
	if (res)
		return res;

	pr_debug("i2c-core: driver [%s] registered\n", driver->driver.name);

	/* Walk the adapters that are already present */
	i2c_for_each_dev(driver, __process_new_driver);

	return 0;
}
EXPORT_SYMBOL(i2c_register_driver);

i2c_bus_type的定义如下

struct bus_type i2c_bus_type = {
	.name		= "i2c",
	.match		= i2c_device_match,
	.probe		= i2c_device_probe,
	.remove		= i2c_device_remove,
	.shutdown	= i2c_device_shutdown,
};
EXPORT_SYMBOL_GPL(i2c_bus_type);

driver_register函数遍历总线的设备链表操作，然后将驱动添加到总线驱动链表中

/**
 * driver_register - register driver with bus
 * @drv: driver to register
 *
 * We pass off most of the work to the bus_add_driver() call,
 * since most of the things we have to do deal with the bus
 * structures.
 */
int driver_register(struct device_driver *drv)
{
	int ret;
	struct device_driver *other;

	BUG_ON(!drv->bus->p);

	if ((drv->bus->probe && drv->probe) ||
	    (drv->bus->remove && drv->remove) ||
	    (drv->bus->shutdown && drv->shutdown))
		printk(KERN_WARNING "Driver '%s' needs updating - please use "
			"bus_type methods\n", drv->name);

	other = driver_find(drv->name, drv->bus);
	if (other) {
		printk(KERN_ERR "Error: Driver '%s' is already registered, "
			"aborting...\n", drv->name);
		return -EBUSY;
	}

	ret = bus_add_driver(drv);
	if (ret)
		return ret;
	ret = driver_add_groups(drv, drv->groups);
	if (ret) {
		bus_remove_driver(drv);
		return ret;
	}
	kobject_uevent(&drv->p->kobj, KOBJ_ADD);

	return ret;
}
EXPORT_SYMBOL_GPL(driver_register);

/**
 * bus_add_driver - Add a driver to the bus.
 * @drv: driver.
 */
int bus_add_driver(struct device_driver *drv)
{
	struct bus_type *bus;
	struct driver_private *priv;
	int error = 0;

	bus = bus_get(drv->bus);
	if (!bus)
		return -EINVAL;

	pr_debug("bus: '%s': add driver %s\n", bus->name, drv->name);

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		error = -ENOMEM;
		goto out_put_bus;
	}
	klist_init(&priv->klist_devices, NULL, NULL);
	priv->driver = drv;
	drv->p = priv;
	priv->kobj.kset = bus->p->drivers_kset;
	error = kobject_init_and_add(&priv->kobj, &driver_ktype, NULL,
				     "%s", drv->name);
	if (error)
		goto out_unregister;

	klist_add_tail(&priv->knode_bus, &bus->p->klist_drivers);// 添加进bus的driver链表中
	if (drv->bus->p->drivers_autoprobe) {
		if (driver_allows_async_probing(drv)) {
			pr_debug("bus: '%s': probing driver %s asynchronously\n",
				drv->bus->name, drv->name);
			async_schedule(driver_attach_async, drv);
		} else {
			error = driver_attach(drv);// 此函数会遍历总线设备链表进行操作
			if (error)
				goto out_unregister;
		}
	}
	module_add_driver(drv->owner, drv);

	error = driver_create_file(drv, &driver_attr_uevent);
	if (error) {
		printk(KERN_ERR "%s: uevent attr (%s) failed\n",
			__func__, drv->name);
	}
	error = driver_add_groups(drv, bus->drv_groups);
	if (error) {
		/* How the hell do we get out of this pickle? Give up */
		printk(KERN_ERR "%s: driver_create_groups(%s) failed\n",
			__func__, drv->name);
	}

	if (!drv->suppress_bind_attrs) {
		error = add_bind_files(drv);
		if (error) {
			/* Ditto */
			printk(KERN_ERR "%s: add_bind_files(%s) failed\n",
				__func__, drv->name);
		}
	}

	return 0;

out_unregister:
	kobject_put(&priv->kobj);
	/* drv->p is freed in driver_release()  */
	drv->p = NULL;
out_put_bus:
	bus_put(bus);
	return error;
}

/**
 * driver_attach - try to bind driver to devices.
 * @drv: driver.
 *
 * Walk the list of devices that the bus has on it and try to
 * match the driver with each one.  If driver_probe_device()
 * returns 0 and the @dev->driver is set, we've found a
 * compatible pair.
 */
int driver_attach(struct device_driver *drv)
{
	return bus_for_each_dev(drv->bus, NULL, drv, __driver_attach);// 遍历总线的设备链表 调用_driver_attach
}
EXPORT_SYMBOL_GPL(driver_attach);

下面来看一看_driver_attach函数，此函数会判断设备和驱动是否匹配，如果匹配就调用驱动的probe函数

static int __driver_attach(struct device *dev, void *data)
{
	struct device_driver *drv = data;

	/*
	 * Lock device and try to bind to it. We drop the error
	 * here and always return 0, because we need to keep trying
	 * to bind to devices and some drivers will return an error
	 * simply if it didn't support the device.
	 *
	 * driver_probe_device() will spit a warning if there
	 * is an error.
	 */

	if (!driver_match_device(drv, dev))
		return 0;

	if (dev->parent)	/* Needed for USB */
		device_lock(dev->parent);
	device_lock(dev);
	if (!dev->driver)
		driver_probe_device(drv, dev);
	device_unlock(dev);
	if (dev->parent)
		device_unlock(dev->parent);

	return 0;
}

这个过程和注册设备过程是一样的

3.3 i2c适配器的侯建

对于i2c适配器，我们不讨论它的注册细节，我们看它是如何构建的

对于三星平台在drivers/i2c/busses/i2c_s3c2410.c文件中构建并注册了i2c适配器，这事三星平台的i2c控制器的驱动

/* i2c bus registration info */

static const struct i2c_algorithm s3c24xx_i2c_algorithm = {
	.master_xfer		= s3c24xx_i2c_xfer,
	.functionality		= s3c24xx_i2c_func,
};


/* s3c24xx_i2c_probe
 *
 * called by the bus driver when a suitable device is found
*/

static int s3c24xx_i2c_probe(struct platform_device *pdev)
{
	struct s3c24xx_i2c *i2c;
	struct s3c2410_platform_i2c *pdata = NULL;
	struct resource *res;
	int ret;

	if (!pdev->dev.of_node) {
		pdata = dev_get_platdata(&pdev->dev);
		if (!pdata) {
			dev_err(&pdev->dev, "no platform data\n");
			return -EINVAL;
		}
	}

	i2c = devm_kzalloc(&pdev->dev, sizeof(struct s3c24xx_i2c), GFP_KERNEL);
	if (!i2c)
		return -ENOMEM;

	i2c->pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);
	if (!i2c->pdata)
		return -ENOMEM;

	i2c->quirks = s3c24xx_get_device_quirks(pdev);
	i2c->sysreg = ERR_PTR(-ENOENT);
	if (pdata)
		memcpy(i2c->pdata, pdata, sizeof(*pdata));
	else
		s3c24xx_i2c_parse_dt(pdev->dev.of_node, i2c);

	strlcpy(i2c->adap.name, "s3c2410-i2c", sizeof(i2c->adap.name));
	i2c->adap.owner = THIS_MODULE;
	i2c->adap.algo = &s3c24xx_i2c_algorithm;// 构建算法
	i2c->adap.retries = 2;
	i2c->adap.class = I2C_CLASS_DEPRECATED;
	i2c->tx_setup = 50;

	init_waitqueue_head(&i2c->wait);

	/* find the clock and enable it */

	i2c->dev = &pdev->dev;
	i2c->clk = devm_clk_get(&pdev->dev, "i2c");
	if (IS_ERR(i2c->clk)) {
		dev_err(&pdev->dev, "cannot get clock\n");
		return -ENOENT;
	}

	dev_dbg(&pdev->dev, "clock source %p\n", i2c->clk);


	/* map the registers */

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	i2c->regs = devm_ioremap_resource(&pdev->dev, res);

	if (IS_ERR(i2c->regs))
		return PTR_ERR(i2c->regs);

	dev_dbg(&pdev->dev, "registers %p (%p)\n",
		i2c->regs, res);

	/* setup info block for the i2c core */

	i2c->adap.algo_data = i2c;
	i2c->adap.dev.parent = &pdev->dev;

	i2c->pctrl = devm_pinctrl_get_select_default(i2c->dev);

	/* inititalise the i2c gpio lines */

	if (i2c->pdata->cfg_gpio) {
		i2c->pdata->cfg_gpio(to_platform_device(i2c->dev));
	} else if (IS_ERR(i2c->pctrl) && s3c24xx_i2c_parse_dt_gpio(i2c)) {
		return -EINVAL;
	}

	/* initialise the i2c controller */

	clk_prepare_enable(i2c->clk);
	ret = s3c24xx_i2c_init(i2c);
	clk_disable(i2c->clk);
	if (ret != 0) {
		dev_err(&pdev->dev, "I2C controller init failed\n");
		return ret;
	}
	/* find the IRQ for this unit (note, this relies on the init call to
	 * ensure no current IRQs pending
	 */

	if (!(i2c->quirks & QUIRK_POLL)) {
		i2c->irq = ret = platform_get_irq(pdev, 0);
		if (ret <= 0) {
			dev_err(&pdev->dev, "cannot find IRQ\n");
			clk_unprepare(i2c->clk);
			return ret;
		}

	ret = devm_request_irq(&pdev->dev, i2c->irq, s3c24xx_i2c_irq, 0,
				dev_name(&pdev->dev), i2c);

		if (ret != 0) {
			dev_err(&pdev->dev, "cannot claim IRQ %d\n", i2c->irq);
			clk_unprepare(i2c->clk);
			return ret;
		}
	}

	ret = s3c24xx_i2c_register_cpufreq(i2c);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to register cpufreq notifier\n");
		clk_unprepare(i2c->clk);
		return ret;
	}

	/* Note, previous versions of the driver used i2c_add_adapter()
	 * to add the bus at any number. We now pass the bus number via
	 * the platform data, so if unset it will now default to always
	 * being bus 0.
	 */

	i2c->adap.nr = i2c->pdata->bus_num;
	i2c->adap.dev.of_node = pdev->dev.of_node;

	platform_set_drvdata(pdev, i2c);

	pm_runtime_enable(&pdev->dev);

	ret = i2c_add_numbered_adapter(&i2c->adap);// 注册了适配器
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to add bus to i2c core\n");
		pm_runtime_disable(&pdev->dev);
		s3c24xx_i2c_deregister_cpufreq(i2c);
		clk_unprepare(i2c->clk);
		return ret;
	}

	pm_runtime_enable(&i2c->adap.dev);

	dev_info(&pdev->dev, "%s: S3C I2C adapter\n", dev_name(&i2c->adap.dev));
	return 0;
}

其中的s3c24xx_i2c_algorithm中的s3c24xx_i2c_xfer 就是通过操作寄存器来通过i2c控制传输数据的

3.4 i2c数据传输

上面介绍了i2c数据传输是通过i2c适配器完成的，下面来分析胰腺癌源码

在i2c驱动中，使用i2c_transfer来传输i2c数据，此函数肯定是通过i2c适配器的算法进行操作的，如下
/**
 * i2c_transfer - execute a single or combined I2C message
 * @adap: Handle to I2C bus
 * @msgs: One or more messages to execute before STOP is issued to
 *	terminate the operation; each message begins with a START.
 * @num: Number of messages to be executed.
 *
 * Returns negative errno, else the number of messages executed.
 *
 * Note that there is no requirement that each message be sent to
 * the same slave address, although that is the most common model.
 */
int i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
	int ret;

	/* REVISIT the fault reporting model here is weak:
	 *
	 *  - When we get an error after receiving N bytes from a slave,
	 *    there is no way to report "N".
	 *
	 *  - When we get a NAK after transmitting N bytes to a slave,
	 *    there is no way to report "N" ... or to let the master
	 *    continue executing the rest of this combined message, if
	 *    that's the appropriate response.
	 *
	 *  - When for example "num" is two and we successfully complete
	 *    the first message but get an error part way through the
	 *    second, it's unclear whether that should be reported as
	 *    one (discarding status on the second message) or errno
	 *    (discarding status on the first one).
	 */

// 调用适配器的算法
	if (adap->algo->master_xfer) {
#ifdef DEBUG
		for (ret = 0; ret < num; ret++) {
			dev_dbg(&adap->dev, "master_xfer[%d] %c, addr=0x%02x, "
				"len=%d%s\n", ret, (msgs[ret].flags & I2C_M_RD)
				? 'R' : 'W', msgs[ret].addr, msgs[ret].len,
				(msgs[ret].flags & I2C_M_RECV_LEN) ? "+" : "");
		}
#endif

		if (in_atomic() || irqs_disabled()) {
			ret = i2c_trylock_adapter(adap);
			if (!ret)
				/* I2C activity is ongoing. */
				return -EAGAIN;
		} else {
			i2c_lock_adapter(adap);
		}

		ret = __i2c_transfer(adap, msgs, num);
		i2c_unlock_adapter(adap);

		return ret;
	} else {
		dev_dbg(&adap->dev, "I2C level transfers not supported\n");
		return -EOPNOTSUPP;
	}
}
EXPORT_SYMBOL(i2c_transfer);

https://blog.csdn.net/weixin_42462202/article/details/100083025