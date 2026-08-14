function blkStruct = slblocks
% CHU_FSAE BMS自定义模块库注册脚本
Browser.Library = 'CHU_FSAE_BMS_Lib'; % 和你的slx文件名完全一致，不要加.slx
Browser.Name    = 'CHU_FSAE BMS Lib'; % 左侧库浏览器显示的名字
Browser.IsFlat  = 1; % 平铺展示模块，适合BMS小库
blkStruct.Browser = Browser;
end