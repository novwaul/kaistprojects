import tensorflow as tf
from tensorflow.examples.tutorials.mnist import input_data
mnist = input_data.read_data_sets("MNIST_data/", one_hot= True)

nb_classes = 10 # from 0 to 9

X = tf.placeholder(tf.float32, [None, 28*28]) # 28*28 pixels
Y = tf.placeholder(tf.float32, [None, nb_classes])

#layer1
W1 = tf.get_variable("W1",shape= [28*28, 256], initializer= tf.contrib.layers.xavier_initializer())
b1 = tf.Variable(tf.random_normal([256]))
L1 = tf.nn.relu(tf.matmul(X, W1) + b1)

#layer2
W2 = tf.get_variable("W2",shape= [256, 256], initializer= tf.contrib.layers.xavier_initializer())
b2 = tf.Variable(tf.random_normal([256]))
L2 = tf.nn.relu(tf.matmul(L1, W2) + b2)

#layer3
W3 = tf.get_variable("W3", shape= [256, 10], initializer= tf.contrib.layers.xavier_initializer())
b3 = tf.Variable(tf.random_normal([10]))
hypothesis = tf.matmul(L2, W3) + b3

cost = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(logits= hypothesis, labels= Y))
train = tf.train.AdamOptimizer(learning_rate= 0.01).minimize(cost)

is_correct = tf.equal(tf.arg_max(hypothesis, 1), tf.arg_max(Y, 1))
accuracy = tf.reduce_mean(tf.cast(is_correct, tf.float32))

training_epochs = 15
batch_size = 100

with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    for epoch in range(training_epochs):
        total_batch = int(mnist.train.num_examples / batch_size)
        avg_cost = 0
        for iter in range(total_batch):
            batch_xs, batch_ys = mnist.train.next_batch(batch_size)
            c, _ = sess.run([cost, train], feed_dict= {X: batch_xs, Y: batch_ys})
            avg_cost += c / total_batch
        print('Epoch:', '%04d' % (epoch + 1), 'cost = ', '{:.9f}'.format(avg_cost))
    print('Accuracy = ', accuracy.eval(session= sess, feed_dict={X: mnist.test.images, Y: mnist.test.labels}))
