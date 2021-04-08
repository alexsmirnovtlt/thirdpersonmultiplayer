# Third Person Multiplayer
    
WIP!   
    
Мультиплейерный шутер от третьего лица.   
В проекте используется:   
- Slate
- Online Sybsystem
- ACharacter (Third Person)
- Gameplay Ability System
- FMOD audio
- Environment Query System + AI
    
Геймплей:   
- 2 команды по 3 игрока
- 1 карта
- Одна из команд должна поставить флаг в определенном месте карты и защищать его
- Раунд заканчивается если
	* одна из команд была уничтожена
	* одна из команд поставила флаг и защищала его в течение определенного времени
	* другая команда сняла установленный первой командой флаг
   
   
Заметки по сборке проекта:    
https://www.fmod.com/resources/documentation-ue4?version=2.0&page=user-guide.html   
Глава 2.4 Compiling the plugin (Optional)  - скачать FMOD for UE4 и добавить файлы для нужной платформы в Plugins\FMODStudio\Binaries\PlatformName (где PlatformName - Win64, Win32, UWP64 и тд)