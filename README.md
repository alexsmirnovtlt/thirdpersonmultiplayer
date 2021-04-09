# Third Person Multiplayer
    
Work in Progress!   
    
Мультиплейерный шутер от третьего лица.   
В проекте используется:   
- Slate   
- Online Subsystem   
- ACharacter (Third Person)    
- Gameplay Ability System   
- FMOD audio   
- Environment Query System + AI   
- Преимущественно С++   
    
Геймплей:   
- 1 карта, 2 команды по 3 игрока
- Игрок одной из команд назначается VIP
- VIP должен провести определенное время в любой из специальных зон
- Раунд заканчивается если
	* одна из команд была полностью уничтожена
	* VIP был уничтожен
	* VIP провел в специальной зоне необходимое время 
   
   
Заметки по сборке проекта:    
- Для компиляции проекта нужно скачать FMOD for UE4 и добавить файлы для нужной платформы в Plugins\FMODStudio\Binaries\PlatformName (где PlatformName - Win64, Win32, UWP64 и тд). Глава 2.4 Compiling the plugin (Optional) - https://www.fmod.com/resources/documentation-ue4?version=2.0&page=user-guide.html   
