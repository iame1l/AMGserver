
delete Web_Users where userid in( select userid from tusers where IsRobot=1)
delete tuserinfo where userid in( select userid from tusers where IsRobot=1)
delete tuserlist where userid in( select userid from tusers where IsRobot=1)
delete tusers where IsRobot=1