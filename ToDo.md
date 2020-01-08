* easy3d:
	- add an application Mapple integrate most of the features in Easy3D;
* glog
	- support for Conditional / Occasional Logging. See http://rpg.ifi.uzh.ch/docs/glog.html
		-  LOG_EVERY_N(INFO, 10) << "Got the " << google::COUNTER << "th cookie";
		-  LOG_IF_EVERY_N(INFO, (size > 1024), 10) << "Got the " << google::COUNTER << "th big cookie";
		-  LOG_FIRST_N(INFO, 20) << "Got the " << google::COUNTER << "th cookie";