import tensorflow as tf
from tensorflow.examples.tutorials.mnist import input_data

mnist = input_data.read_data_sets("MNIST_data/", one_hot= True)
learning_rate= 0.001
training_epochs= 15
batch_size= 100

class MyModel: #Learning Model
    def __init__(self, sess, name):
        self.sess = sess
        self.name = name
        self._build_network()
    def _build_network(self):
        with tf.variable_scope(self.name):
            self.training = tf.placeholder(tf.bool)

            self.X = tf.placeholder(tf.float32, [None, 28*28])
            self.Y = tf.placeholder(tf.float32, [None, 10])
            X_img = tf.reshape(self.X, [-1, 28, 28, 1])
            #Convolution & Pooling layer1
            conv1 = tf.layers.conv2d(inputs= X_img, filters= 32, kernel_size= [3,3], padding= "SAME", activation= tf.nn.relu)
            pool1 = tf.layers.max_pooling2d(inputs= conv1, pool_size= [2,2], padding= "SAME", strides= 2)
            dropout1 = tf.layers.dropout(inputs= pool1, rate= 0.3, training= self.training)
            #Convolution & Pooling layer1
            conv2 = tf.layers.conv2d(inputs= dropout1, filters= 64, kernel_size= [3,3], padding= "SAME", activation= tf.nn.relu)
            pool2 = tf.layers.max_pooling2d(inputs= conv2, pool_size= [2,2], padding= "SAME", strides= 2)
            dropout2 = tf.layers.dropout(inputs= pool2, rate= 0.3, training= self.training)
            #Convolution & Pooling layer1
            conv3 = tf.layers.conv2d(inputs= dropout2, filters= 128, kernel_size= [3,3], padding= "SAME", activation= tf.nn.relu)
            pool3 = tf.layers.max_pooling2d(inputs= conv3, pool_size= [2,2], padding= "SAME", strides= 2)
            dropout3 = tf.layers.dropout(inputs= pool3, rate= 0.3, training= self.training)
            #Dense layer
            flat = tf.reshape(dropout3, [-1, 128*4*4])
            dense = tf.layers.dense(inputs= flat, units= 625, activation= tf.nn.relu)
            dropout4 = tf.layers.dropout(inputs= dense, rate= 0.5, training= self.training)

            self.logits = tf.layers.dense(inputs= dropout4, units= 10)

        #cost & optimizer
        self.cost = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(logits= self.logits, labels= self.Y))
        self.optimizer = tf.train.AdamOptimizer(learning_rate= learning_rate).minimize(self.cost)

        #accuracy
        correct_prediction = tf.equal(tf.argmax(self.logits, 1), tf.argmax(self.Y, 1))
        self.accuracy = tf.reduce_mean(tf.cast(correct_prediction, tf.float32))

    def predict(self, x_test, y_test, training= False):
        return self.sess.run(self.accuracy, feed_dict= {self.X: x_test, self.Y: y_test, self.training: training})

    def train(self, x_data, y_data, training= True):
        return self.sess.run([self.cost, self.optimizer], feed_dict={self.X: x_data, self.Y: y_data, self.training: training})

    def get_accuracy(self, x_test, y_test, training= False):
        return self.sess.run(self.accuracy, feed_dict= {self.X: x_test, self.Y: y_test, self.training: training})

# Do training
sess = tf.Session()
model1 = MyModel(sess, "model1")

sess.run(tf.global_variables_initializer())

for epoch in range(training_epochs):
    avg_cost = 0
    total_batch = int(mnist.train.num_examples / batch_size)

    for i in range(total_batch):
        batch_xs, batch_ys = mnist.train.next_batch(batch_size)
        c, _ = model1.train(batch_xs, batch_ys)
        avg_cost += c / total_batch

    print("Epoch:", '%04d' % (epoch + 1), 'cost =', '{:.9f}'.format(avg_cost))

print("Accuracy:", model1.get_accuracy(mnist.test.images, mnist.test.labels))
