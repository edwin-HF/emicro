# EMicro


### 简介

    基于注解的php框架，支持自定义注解解析

### 环境

* Linux
* php 最小版本7.4

### 部署
* 进入 emicro 文件夹
* 执行
```
 phpize && ./configure --with-php-config=yourpath/php-config && make && make install
```
* php.ini 文件 新增
```
 extension=emicro.so
```
* nginx 配置
```
    location / {
        if (!-e $request_filename) {
            rewrite  ^(.*)$  /index.php?s=$1  last;
            break;
        }
    }

```

### 推荐项目目录
```
    project
        |--application
            |--annotation【注解解析目录】
                |--Cache.php
                |--...
            |--config【配置文件目录】
                |--system.php【返回数组格式】
                |--...
            |--controller【控制器目录】
                |--Index.php
                |--...
        |--public
            |--index.php
        |--vendor【composer 安装的扩展，自动生成】
        |--composer.json【composer 扩展配置文件，自动生成】
```

### 快速开始

    案例中 orm 集成的是think-orm（composer require topthink/think-orm）,实际情况请根据自己的喜好集成

* public/index.php

```
<?php

    use EMicro\Application;

    // composer 扩展自动加载
    require_once "../vendor/autoload.php";

    // 获取引用实例
    $application = Application::getInstance();

    // 设置注解解析目录，不设置默认为【application/annotation】
    // $application->annotationNamespace('');
    // 设置控制器目录，不设置默认为【application/controller】
    // $application->dispatcherNamespace('');

    // 集成think-orm , 配置数据库
    \think\facade\Db::setConfig(config('database'));

    // 开始运行
    $application->run();

```

* application/controller/Index.php

```
<?php


    namespace controller;

    use EMicro\Request;

    /**
    * @Route(index)【路由地址，不设置默认为类名】
    */
    class Index
    {

        /**
        * @Route(hello)【路由地址，不设置默认为方法名，当前访问地址为：http://domain/index/hello】
        */
        public function index(Request $request){
            return 'hello EMicro!';
        }

    }

```

    接下来就可以愉快的玩耍了...

### 文档

```

 EMicro\Application
    |-- public static function getInstance(){}
    |-- public function dispatcherNamespace($dispatcherDir){}
    |-- public function run(){}
    |-- public function getAppPath(){}

```

```

 EMicro\Dispatcher
    |-- public static function getInstance(){}
    |-- public function controller(){}
    |-- public function method(){}

```

```

 EMicro\Factory
    |-- public static function call($class,$method,$params){}

```

```

 EMicro\Request
    |-- public static function get($param = '', $default=''){}
    |-- public static function post($param = '', $default=''){}
    |-- public static function input($param = '', $default=''){}
    |-- public static function server($param = '', $default=''){}
    |-- public static function cookie($param = '', $default=''){}
    |-- public static function files($param = ''){}
    |-- public static function header($param = '', $default=''){}
    |-- public static function isPost(){}
    |-- public static function isGet(){}
    |-- public static function isPut(){}
    |-- public static function isDelete(){}
    |-- public static function isPatch(){}
    |-- public static function isHead(){}

```

```
    全局函数 function config($param = '',$default=''){}

    内置注解 @Route(value),应用与控制器类和方法，如果不设置则默认使用【控制器类名称/控制器方法名称】

```
* 自定义注解
```
    比如在控制器上新增注解@Auth(params),则需要在application/annotation 中新增Auth类，实现run 方法，run方法接收2个参数，一个是注解中的参数，一个方法的返回值（适用后置注解）

    前置注解【在方法执行前执行】
        @Auth(params) 或 @Auth(params)!before

    后置注解【在方法执行后执行】
        @Auth(params)!after

    自己实现的类（除了控制器中的类），比如自己实现的service或logic 也可使用注解，只是调用方式由


                 $service = new Service();                          
                 $returnData = $service->method($param1, $param2);   

                            
                                        |
                                        |变
                                        |成
                                      \ | /
                                       \|/
                

     $returnData = EMicro\Factory::call(Service::class, "method", [$param1,$param2]) 

```
* 注解解析类例子
```
    <?php


    namespace annotation;


    class Auth
    {

        public function run($params, $returnData){
            var_dump('auth annotation');
            var_dump($params);
            var_dump($returnData);
        }

    }
```

* 项目没有集成视图，可以自己实现，以下是个视图注解的案例，可参考
```
<?php


    namespace annotation;


    use EMicro\Application;

    class View
    {

        public function run($params,$data){
            header('Content-Type:text/html');
            extract($data);
            die(require Application::getInstance()->getAppPath().'/view/'.$params.'.phtml');
        }

    }
```

* application/controller/Index.php
```
<?php


namespace controller;

use EMicro\Request;

/**
 * @Route(index)
 */
class Index
{

    /**
     * @Route(hello)
     * @View(index/index)!after
     */
    public function index(Request $request){

        return  [
            [
                'id'   => 1,
                'name' => 'LiSi',
                'age'  => 12
            ],
            [
                'id'   => 2,
                'name' => 'LaoWang',
                'age'  => 13
            ],
        ];

    }

}
```


* application/view/index/index.phtml
```
<h1>
    <?=$title??'' ?>

    <table border="1px">
        <thead>
            <tr>
                <th>ID</th>
                <th>name</th>
                <th>age</th>
            </tr>
        </thead>

            <?php foreach ($user??[] as $value) { ?>
                <tr>
                    <th><?=$value['id']; ?></th>
                    <th><?=$value['name']; ?></th>
                    <th><?=$value['age']; ?></th>
                </tr>
            <?php } ?>

        <tbody>

        </tbody>
    </table>

</h1>

```