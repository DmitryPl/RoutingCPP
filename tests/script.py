import Tour

Point_class = Tour.Point()
Point_class.matrix_id = 2;
Point_class.point = (1.0, 2.0)
Point_class.address = "adress"

Window_class = Tour.Window()
Window_class.window = (12, 14)

Storage_class = Tour.Storage()
Storage_class.name = "storage"
Storage_class.load = 120
Storage_class.skills = ['a', 'b', 'c']
Storage_class.location = Point_class
Storage_class.work_time = Window_class

Cost_class = Tour.Cost()
Cost_class.start = 0.7
Cost_class.second = 0.9
Cost_class.meter = 1.1

Courier_class = Tour.Courier()
Courier_class.name = 'name'
Courier_class.profile = 'profile'
Courier_class.cost = Cost_class
Courier_class.value = [3, 5, 8]
Courier_class.skills = ['a', 'b', 'c']
Courier_class.max_distance = 1000
Courier_class.work_time = Window_class
Courier_class.start_location = Point_class
Courier_class.end_location = Point_class

Matrix_class = Tour.Matrix()
Matrix_class.profile = "profile"
Matrix_class.distance = [[3, 5, 8]]
Matrix_class.travel_time = [[3, 5, 8]]

Job_class = Tour.Job()
Job_class.job_id = '12345'
Job_class.storage = Storage_class
Job_class.delay = 30
Job_class.value = [3, 5, 8]
Job_class.skills = ['a', 'b', 'c']
Job_class.location = Point_class
Job_class.time_windows = [Window_class, Window_class, Window_class]

Route_class = Tour.Route()
Route_class.storage = Storage_class
Route_class.courier = Courier_class
Route_class.matrix = Matrix_class
Route_class.start_time = 10
Route_class.jobs = [Job_class, Job_class, Job_class]
Route_class.travel_time = 100
Route_class.distance = 500
Route_class.cost = 3.5

Tour_main_class = Tour.Tour()
Tour_main_class.storage = Storage_class
Tour_main_class.routes = [Route_class, Route_class, Route_class]
Tour_main_class.unassigned_jobs = [Job_class, Job_class, Job_class]

obj = Tour.ClassicProblem()
Tour.go_print(obj)

# Tour_main_class.problem = 'Problem'
#ret = Tour_main_class.improve_tour(False, False)

# print('ret')
