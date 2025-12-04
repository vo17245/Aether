## 资源管理

由于vulkan中创建descriptor set是廉价操作，DeviceDescriptorPool的使用方式是每帧都创建新的descriptor set
在每帧开始时调用DeviceDescriptorPool.Clear() 删除所有descriptor set
DeviceDescriptorPool 由多个DescriptorPool组成，每个pool容量为1000，在耗尽时会自动创建新的来扩容